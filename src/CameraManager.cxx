#include "CameraManager.h"

#include <stdexcept>
#include <Ogre.h>
#include <OgreCameraMan.h>

namespace
{
    const char* const kFixedCameraName  = "FixedCamera";
    const char* const kBallCameraName   = "BallCamera";
    const char* const kGlobalCameraName = "MainCamera";
    const char* const kManCameraName    = "ManCamera";

    const float kMinGlobalCameraY       = 0.5f;  

    const Ogre::Vector3 kBallCameraOffset( 0.0f, 0.8f, -2.5f );
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
    this->m_CamMan = new OgreBites::CameraMan( global-> getParentSceneNode( ) );
    this->m_CamMan->setStyle( OgreBites::CS_ORBIT );
    
    this->m_Viewport = window->addViewport( global );
    this->select( Global );
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

    Ogre::SceneNode* node = this->m_Cameras[ Global ]->getParentSceneNode( );
    Ogre::Vector3 pos = node->getPosition( );
    if( pos.y < kMinGlobalCameraY )
    {
        pos.y = kMinGlobalCameraY;
        node->setPosition( pos );
        node->lookAt( Ogre::Vector3::ZERO, Ogre::Node::TS_WORLD );
    }
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
setFollowedNode( Ogre::SceneNode* node )
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

    return( this->_isGlobalActive( ) && this->m_CamMan->mouseWheelRolled( evt ) );
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