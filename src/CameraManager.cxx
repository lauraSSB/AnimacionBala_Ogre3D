#include "CameraManager.h"

#include <stdexcept>
#include <Ogre.h>
#include <OgreCameraMan.h>
#include <iostream>

namespace
{
    const char* const kFixedCameraName  = "FixedCamera";
    const char* const kBallCameraName   = "BallCamera";
    const char* const kGlobalCameraName = "MainCamera";
    const char* const kManCameraName    = "ManCamera";

    const float kMinGlobalCameraY = 0.1f;
    const Ogre::Vector3 kWorldMin( -300.0f, kMinGlobalCameraY, -300.0f );
    const Ogre::Vector3 kWorldMax(  300.0f, 300.0f,             300.0f );
    const Ogre::Vector3 kBallCameraOffset( 0.0f, 0.8f, -2.5f );

    const char* const kForbiddenZoneName = "Zona_Prohibida";

    bool rayHitsTriangle(
        const Ogre::Vector3& origin, const Ogre::Vector3& dir,
        const Ogre::Vector3& a, const Ogre::Vector3& b, const Ogre::Vector3& c
    )
    {
        const Ogre::Vector3 edge1 = b - a;
        const Ogre::Vector3 edge2 = c - a;
        const Ogre::Vector3 p     = dir.crossProduct( edge2 );
        const float det           = edge1.dotProduct( p );
        if( Ogre::Math::Abs( det ) < 1e-9f )
            return false;

        const float inv = 1.0f / det;
        const Ogre::Vector3 t = origin - a;
        const float u = t.dotProduct( p ) * inv;
        if( u < 0.0f || u > 1.0f )
            return false;

        const Ogre::Vector3 q = t.crossProduct( edge1 );
        const float v = dir.dotProduct( q ) * inv;
        if( v < 0.0f || u + v > 1.0f )
            return false;

        return edge2.dotProduct( q ) * inv > 0.0f;
    }
}

//--------------------------------------------------------------------------
CameraManager::
~CameraManager( )
{
    delete this->m_CamMan;
}

//--------------------------------------------------------------------------
void CameraManager::
build(
    Ogre::SceneManager* sceneMgr, 
    Ogre::RenderWindow* window, 
    const std::string& followedNodeName
)
{
    this->m_FollowedNode = sceneMgr->getSceneNode( followedNodeName );

    this->m_Cameras.resize( Count );
    this->m_Cameras[ Fixed ]    = this->_requireCamera( sceneMgr, kFixedCameraName );
    this->m_Cameras[ Ball ]     = this->_requireCamera( sceneMgr, kBallCameraName );
    this->m_Cameras[ Man ]      = this->_requireCamera( sceneMgr, kManCameraName );
    this->m_BallCameraNode      = this->m_Cameras[ Ball ]->getParentSceneNode( );
    this->m_BallCameraNode->setFixedYawAxis( true );
    this->m_Cameras[ Global ]   = this->_requireCamera( sceneMgr, kGlobalCameraName );

    auto* global = this->m_Cameras[ Global ];
    Ogre::SceneNode* globalNode = global->getParentSceneNode( );
    globalNode->setFixedYawAxis( true );

    // Vista inicial: la posición del .scene, mirando al centro
    const Ogre::Vector3 startPos = globalNode->getPosition( );
    const float startDist = startPos.length( );

    this->m_CamMan = new OgreBites::CameraMan( globalNode );
    this->m_CamMan->setStyle( OgreBites::CS_ORBIT );
    this->m_CamMan->setYawPitchDist(
        Ogre::Math::ATan2( startPos.x, startPos.z ),   
        Ogre::Math::ASin( startPos.y / startDist ),      
        startDist                                        
    );

    this->m_Viewport = window->addViewport( global );
    this->select( Global );

    this->m_LastGoodPosition = globalNode->getPosition( );
    this->_buildForbiddenZone( sceneMgr );
}

//--------------------------------------------------------------------------
// Cambia la cámara activa
void CameraManager::
select( CameraId id )
{
    this->m_Active = static_cast< std::size_t >( id ) % this->m_Cameras.size( );
    this->m_Viewport->setCamera( this->m_Cameras[ this->m_Active ] );
}

//--------------------------------------------------------------------------
// Pasa a la cámara siguiente, y de la última vuelve a la primera
void CameraManager::
selectNext( )
{
    this->select(
        static_cast< CameraId >( ( this->m_Active + 1 ) % this->m_Cameras.size( ) )
        );
}

//--------------------------------------------------------------------------
// Pasa a la cámara anterior, y de la primera vuelve a la última
void CameraManager::
selectPrevious( )
{
    this->select(
        static_cast< CameraId >(
            ( this->m_Active + this->m_Cameras.size( ) - 1 ) % this->m_Cameras.size( )
            )
        );
}

//--------------------------------------------------------------------------
// Indica si la Camara id esta activa
bool CameraManager::
isActive( CameraId id ) const
{
    return( this->m_Active == static_cast< std::size_t >( id ) );
}

//--------------------------------------------------------------------------
// Cámara que se está usando ahora
Ogre::Camera* CameraManager::
getActiveCamera( ) const
{
    return( this->m_Cameras[ this->m_Active ] );
}

//--------------------------------------------------------------------------
// Pone la cámara de la bola detrás de ella y la apunta hacia ella
void CameraManager::
update( )
{
    if( this->m_Active != Ball )
        return;

    const Ogre::Vector3 target = this->m_FollowedNode->_getDerivedPosition( );
    this->m_BallCameraNode->setPosition( target + kBallCameraOffset );
    this->m_BallCameraNode->lookAt( target, Ogre::Node::TS_WORLD );
}

//--------------------------------------------------------------------------
// Reenvía el movimiento del ratón al CameraMan solo si la cámara global está activa
bool CameraManager::
mouseMoved( const OgreBites::MouseMotionEvent& evt )
{
    if( !this->_isGlobalActive( ) )
        return( false );

    bool handled = this->m_CamMan->mouseMoved( evt );

    this->_keepGlobalCameraInWorld( );
    this->_keepGlobalCameraOutOfZone( );
    return( handled );
}

//--------------------------------------------------------------------------
// Reenvía el click al CameraMan solo si la cámara global está activa
bool CameraManager::
mousePressed( const OgreBites::MouseButtonEvent& evt )
{
    return( this->_isGlobalActive( ) && this->m_CamMan->mousePressed( evt ) );
}

//--------------------------------------------------------------------------
// Reenvía la liberación del click al CameraMan solo si la cámara global está activa
bool CameraManager::
mouseReleased( const OgreBites::MouseButtonEvent& evt )
{
    return( this->_isGlobalActive( ) && this->m_CamMan->mouseReleased( evt ) );
}

//--------------------------------------------------------------------------
// Con el modo activo, la rueda cambia de cámara en vez de hacer zoom
void CameraManager::
setCycling( bool enabled )
{
    this->m_Cycling = enabled;
}

//--------------------------------------------------------------------------
//Cambia el nodo que sigue la cámara de la bola
void CameraManager::
setFollowedNode( Ogre::Node* node )
{
    this->m_FollowedNode = node;
}

//--------------------------------------------------------------------------
// Si está activo, la rueda no hace zoom (la usa otra función)
void CameraManager::
setWheelBlocked( bool blocked )
{
    this->m_WheelBlocked = blocked;
}

//--------------------------------------------------------------------------
// Con el modo activo, la rueda cambia de cámara: hacia arriba avanza, hacia
// abajo retrocede. Si no, hace zoom, pero solo si la cámara global está activa
bool CameraManager::
mouseWheelRolled( const OgreBites::MouseWheelEvent& evt )
{
    if( this->m_Cycling )
    {
        if( evt.y > 0 )
            this->selectNext( );
        else if( evt.y < 0 )
            this->selectPrevious( );
        return( true );
    }

    if( this->m_WheelBlocked ) 
        return( false );

    if( !this->_isGlobalActive( ) )
        return( false );
    bool handled = this->m_CamMan->mouseWheelRolled( evt );
    this->_keepGlobalCameraInWorld( );
    this->_keepGlobalCameraOutOfZone( );
    return( handled );
    
}

//--------------------------------------------------------------------------
// Busca una cámara por nombre y avisa si no existe en la escena
Ogre::Camera* CameraManager::
_requireCamera( Ogre::SceneManager* sceneMgr, const std::string& name )
{
    if( !sceneMgr->hasCamera( name ))
        throw std::runtime_error( "CameraManager: camera " + name + " not found" );
    auto* cam = sceneMgr->getCamera( name );
    cam->setAutoAspectRatio( true );
    return( cam );
}

//--------------------------------------------------------------------------
// Indica si la cámara activa es la global
bool CameraManager::
_isGlobalActive( ) const
{
    return( this->m_Active == Global && this->m_CamMan != nullptr );
}

//--------------------------------------------------------------------------
// Mantiene la cámara global dentro de la caja del mundo y la reapunta al centro
void CameraManager::
_keepGlobalCameraInWorld( )
{
    Ogre::SceneNode* node = this->m_Cameras[ Global ]->getParentSceneNode( );
    const Ogre::Vector3 pos = node->getPosition( );

    const Ogre::Vector3 clamped(
        Ogre::Math::Clamp( pos.x, kWorldMin.x, kWorldMax.x ),
        Ogre::Math::Clamp( pos.y, kWorldMin.y, kWorldMax.y ),
        Ogre::Math::Clamp( pos.z, kWorldMin.z, kWorldMax.z )
        );

    if( clamped != pos )
    {
        node->setPosition( clamped );
        node->lookAt( Ogre::Vector3::ZERO, Ogre::Node::TS_WORLD );
    }
}

//--------------------------------------------------------------------------
// Copia los triángulos de la malla de la zona prohibida (ya en coordenadas del mundo)
void CameraManager::
_buildForbiddenZone( Ogre::SceneManager* sceneMgr )
{
    if( !sceneMgr->hasEntity( kForbiddenZoneName ) )
        return;

    Ogre::Entity* entity = sceneMgr->getEntity( kForbiddenZoneName );
    Ogre::MeshPtr mesh   = entity->getMesh( );
    const Ogre::Matrix4 world = entity->getParentSceneNode( )->_getFullTransform( );

    for( unsigned s = 0; s < mesh->getNumSubMeshes( ); ++s )
    {
        Ogre::SubMesh* sub = mesh->getSubMesh( s );
        Ogre::VertexData* vertexData = sub->useSharedVertices ? mesh->sharedVertexData : sub->vertexData;

        // Posiciones
        const Ogre::VertexElement* element =
            vertexData->vertexDeclaration->findElementBySemantic( Ogre::VES_POSITION );
        Ogre::HardwareVertexBufferSharedPtr vbuf =
            vertexData->vertexBufferBinding->getBuffer( element->getSource( ) );

        std::vector< Ogre::Vector3 > positions( vertexData->vertexCount );
        unsigned char* vertex = static_cast< unsigned char* >(
            vbuf->lock( Ogre::HardwareBuffer::HBL_READ_ONLY )
            );
        for( std::size_t v = 0; v < positions.size( ); ++v )
        {
            float* p;
            element->baseVertexPointerToElement( vertex + v * vbuf->getVertexSize( ), &p );
            positions[ v ] = world * Ogre::Vector3( p[ 0 ], p[ 1 ], p[ 2 ] );
        }
        vbuf->unlock( );

        // Índices
        Ogre::IndexData* indexData = sub->indexData;
        Ogre::HardwareIndexBufferSharedPtr ibuf = indexData->indexBuffer;
        const bool is32 = ( ibuf->getType( ) == Ogre::HardwareIndexBuffer::IT_32BIT );
        void* raw = ibuf->lock( Ogre::HardwareBuffer::HBL_READ_ONLY );
        for( std::size_t k = indexData->indexStart;
             k + 2 < indexData->indexStart + indexData->indexCount; k += 3 )
        {
            for( int corner = 0; corner < 3; ++corner )
            {
                const std::size_t index = is32
                    ? static_cast< unsigned int* >( raw )[ k + corner ]
                    : static_cast< unsigned short* >( raw )[ k + corner ];
                this->m_ZoneTriangles.push_back( positions[ index ] );
            }
        }
        ibuf->unlock( );
    }
    std::cout << "[Cam] zona prohibida: " << this->m_ZoneTriangles.size( ) / 3
              << " triangulos" << std::endl;
}

//--------------------------------------------------------------------------
// Rayo hacia arriba desde el punto: si cruza la malla un número impar de veces, está dentro
bool CameraManager::
_isInsideForbiddenZone( const Ogre::Vector3& point ) const
{
    // Dirección casi vertical, para no pegarle exacto a una arista
    const Ogre::Vector3 dir = Ogre::Vector3( 0.0013f, 1.0f, 0.0007f ).normalisedCopy( );

    int crossings = 0;
    for( std::size_t i = 0; i + 2 < this->m_ZoneTriangles.size( ); i += 3 )
        if( rayHitsTriangle( point, dir,
                this->m_ZoneTriangles[ i ],
                this->m_ZoneTriangles[ i + 1 ],
                this->m_ZoneTriangles[ i + 2 ] ) )
            ++crossings;

    return( crossings % 2 == 1 );
}

//--------------------------------------------------------------------------
// Si la cámara global entra en la zona prohibida, vuelve a su última posición buena
void CameraManager::
_keepGlobalCameraOutOfZone( )
{
    if( this->m_ZoneTriangles.empty( ) )
        return;

    Ogre::SceneNode* node = this->m_Cameras[ Global ]->getParentSceneNode( );
    const Ogre::Vector3 position = node->getPosition( );

    if( this->_isInsideForbiddenZone( position ) )
    {
        node->setPosition( this->m_LastGoodPosition );
        node->lookAt( Ogre::Vector3::ZERO, Ogre::Node::TS_WORLD );
    }
    else
        this->m_LastGoodPosition = position;
}