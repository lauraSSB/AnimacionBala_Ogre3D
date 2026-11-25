# include <iostream>
# include "Application.h"

# include <filesystem>
# include <vector>
# include <sstream>

# include <OgreOverlaySystem.h>
# include <Ogre.h>
# include <OgreCameraMan.h>
# include <OgreBone.h>

namespace
{
    const std::vector< std::string > kProjectileNames = { 
        "Bala", "yellow_ball", "rubber_duck", "banana", "cerveza" 
    };
    const std::vector< std::string > kCharacterNames =
    {
        "Lanzador_Atleta", "Lanzador_Warrior", "Lanzador_Cleric"
    };

    const float kLaunchSpeed            = 12.0f;
    const float kLaunchUpwardRatio      = 0.84f;
    const float kLaunchForwardOffset    = 0.5f;
    const float kMinThrowAnimSpeed      = 0.6f;   
    const float kMaxThrowAnimSpeed      = 1.6f;   
    const float kCircleRadius           = 1.0675f;   
    const float kReleaseTime            = 2.21f;
    const float kPalmDist               = 0.2f;  
    const float kSpinPerSpeed           = 0.6f;

}
//---------------------------------------------------------------------------------
Application::
Application( const std::string& app_name, const std::string& main_dir )
    : TContext ( app_name ),
      TListener ( )
{
    auto p = 
        std::filesystem::canonical( std::filesystem::path( main_dir ) ).
        parent_path();
    p /= "resources.cfg";
    this->m_ResourcesFile = p.string( );
}

//---------------------------------------------------------------------------------
void Application::
loadResources( )
{
    this->enableShaderCache( );
    Ogre::ConfigFile cf = Ogre::ConfigFile( );
    cf.loadDirect( this-> m_ResourcesFile );

    auto res_mgr = Ogre::ResourceGroupManager::getSingletonPtr( );
    auto settings = cf.getSettingsBySection( );
    for( auto sIt = settings.begin( ); sIt != settings.end( ); ++sIt )
        for( auto fIt = sIt->second.begin( ); fIt != sIt->second.end( ); ++fIt)
            res_mgr->addResourceLocation( fIt->second, fIt->first, sIt->first, false );

    try
    {
        res_mgr->initialiseAllResourceGroups( );
        res_mgr->loadResourceGroup( "General" );
    }
    catch( ... )
    {
        // Do nothing
    }
}

//----------------------------------------------------------------------------------
void Application::
setup( )
{
    this->_initSceneManager( );
    this->_loadScene( );
    this->_configurePhysics( );
    this->_configureCamera( );
        std::cout << "[Render] " << this->getRoot( )->getRenderSystem( )->getName( ) << std::endl;
}

//----------------------------------------------------------------------------------
bool Application::
frameStarted( const Ogre::FrameEvent& evt )
{
    this->m_Thrower.update( evt.timeSinceLastFrame );
    return( this->TContext::frameStarted( evt ) );
}

//----------------------------------------------------------------------------------
void Application::
frameRendered( const Ogre::FrameEvent& evt )
{
    this->m_Physics.update( evt.timeSinceLastFrame );
    this->m_CameraMgr.update( );
    this->_updateResult( );

    const bool manActive = this->m_CameraMgr.isActive( CameraManager::Man );
    
    if( manActive )
        this->m_Shot.update( evt.timeSinceLastFrame );

    this->m_MeterView.setVisible( manActive );
    this->m_MeterView.setCursor( this->m_Shot.getCursor( ) );
}

//----------------------------------------------------------------------------------
bool Application::
keyPressed( const OgreBites::KeyboardEvent& evt )
{
    if( evt.keysym.sym == OgreBites::SDLK_ESCAPE){
        this->getRoot( )->queueEndRendering( );
    }

    if( evt.keysym.sym == '1' ) this->m_CameraMgr.select( CameraManager::Fixed );
    if( evt.keysym.sym == '2' ) this->m_CameraMgr.select( CameraManager::Ball );
    if( evt.keysym.sym == '3' ) this->m_CameraMgr.select( CameraManager::Global );
    if( evt.keysym.sym == '4' ) this->m_CameraMgr.select( CameraManager::Man );
    if( evt.keysym.sym == 'c' ) this->m_CameraMgr.setCycling( true );

    if( evt.keysym.sym == 'p' && !evt.repeat )
        this->m_Thrower.start( [ ]( ) { } );

    if( evt.keysym.sym == 'r' && !this->m_Thrower.isBusy( ) )
        this->_selectProjectile( this->m_CurrentProjectile );

    if( evt.keysym.sym == 'n' && !evt.repeat )
        this->_nextCharacter( );

    if( evt.keysym.sym == ' ' && !evt.repeat && this->m_CameraMgr.isActive( CameraManager::Man ) )
    {
        const float power = this->m_Shot.getPower( );
        std::cout<< "[Shoot] fuerza " << power << std::endl;

        this->m_Thrower.setSpeed(
            kMinThrowAnimSpeed + ( kMaxThrowAnimSpeed - kMinThrowAnimSpeed ) * power
            );

        this->_shootBall( ShootController::powerToSpeed( power ) );
    }

    if( evt.keysym.sym == 'o' )
    {
        this->m_ObjectKeyHeld = true;
        this->m_CameraMgr.setWheelBlocked( true );
    }

    return( true );
}
//----------------------------------------------------------------------------------
// Al soltar la tecla C se deja de cambiar de cámara con la rueda
bool Application::
keyReleased( const OgreBites::KeyboardEvent& evt )
{
    if( evt.keysym.sym == 'c' ){
        this->m_CameraMgr.setCycling( false );
    }

    if( evt.keysym.sym == 'o'){
        this->m_ObjectKeyHeld = false;
        this->m_CameraMgr.setWheelBlocked( false );
    }
    return ( true );
}

//----------------------------------------------------------------------------------
// Con la tecla O oprimida, la rueda cambia de proyectil
bool Application::
mouseWheelRolled( const OgreBites::MouseWheelEvent& evt )
{
    if( !this->m_ObjectKeyHeld )
        return ( false );

    if( evt.y > 0 )
        this->_cycleProjectile( 1 );
    else if( evt.y < 0 )
        this->_cycleProjectile( -1 );
    return ( true );
}


// //----------------------------------------------------------------------------------
// // Click derecho: lanza la bola, pero solo si la cámara activa es la del lanzador
// bool Application::
// mousePressed( const OgreBites::MouseButtonEvent& evt )
// {
//     if( evt.button == OgreBites::BUTTON_RIGHT && this->m_CameraMgr.isActive( CameraManager::Man ) ){
//         this->_shootBall( );
//         return( true );
//     }
//     return ( false );
// }

//----------------------------------------------------------------------------------
// Lanza la bola desde la cámara, en la dirección en que mira
void Application::
_shootBall( float speed )
{
    Ogre::Camera* cam = this->m_CameraMgr.getActiveCamera( );

    Ogre::Vector3 forward = cam->getDerivedDirection( );
    forward.y = 0;
    forward.normalise( );

    const Ogre::Vector3 direction
        = ( forward + Ogre::Vector3::UNIT_Y * kLaunchUpwardRatio ).normalisedCopy( );

    const Ogre::Vector3 spinAxis = direction.crossProduct( Ogre::Vector3::UNIT_Y ).normalisedCopy( );
    const Ogre::Vector3 spin     = spinAxis * ( kSpinPerSpeed * speed );

    const Ogre::Vector3 origin
        = cam->getDerivedPosition( ) + cam->getDerivedDirection( ) * kLaunchForwardOffset;

    if( this->m_Thrower.isBusy( ) )
        return;

    this->m_ResultView.hide( );

    if( this->m_HeldEntity == nullptr )
        this->_selectProjectile( this->m_CurrentProjectile );

    const std::string name       = kProjectileNames[ this->m_CurrentProjectile ];
    const Ogre::Vector3 velocity = direction * speed;

    this->m_Thrower.start( [ this, name, velocity, spin ]( )
    {
        // El origen es donde esté el objeto en la mano en ese instante
        Ogre::Vector3 origin = Ogre::Vector3::ZERO;
        if( this->m_HeldEntity != nullptr )
            origin = this->m_HeldEntity->getParentNode( )->_getDerivedPosition( );

        this->_dropProjectile( );
        this->m_Physics.launch( name, origin, velocity, spin );
        this->m_TrackedName = name;
        this->m_Airborne    = false;
        this->m_Tracking    = true;
    } );
}

//----------------------------------------------------------------------------------
// Deja activo solo el proyectil actual y oculta lo demas. SObre el actual se hace la física
void Application::
_selectProjectile( std::size_t index )
{
    this->m_Tracking = false;
    this->_dropProjectile( );
    for( std::size_t i = 0; i < kProjectileNames.size( ); ++i )
    {
        const bool active = ( i == index );
        this->m_Physics.setBodyEnabled( kProjectileNames[ i ], active );
        this->m_SceneMgr->getEntity( kProjectileNames[ i ] )->setVisible( active );
    }

    this->m_CurrentProjectile = index;
    Ogre::Entity* entity = this->m_SceneMgr->getEntity( kProjectileNames[ index ] );
    this->m_CameraMgr.setFollowedNode( entity->getParentSceneNode( ) );

    this->_holdProjectile( entity );
}

//----------------------------------------------------------------------------------
// Avanza o retrocede en la lista de proyectiles, dando la vuelta al final
void Application::
_cycleProjectile( int step )
{
    const int count = static_cast< int >( kProjectileNames.size( ) );
    const int next = ( static_cast< int >( this->m_CurrentProjectile ) + step + count ) % count;
    this->_selectProjectile( static_cast< std::size_t >( next ) );
}

//----------------------------------------------------------------------------------
// Pega el proyectil al hueso Bola del atleta
void Application::
_holdProjectile( Ogre::Entity* entity )
{
    Ogre::Entity*    thrower = this->m_Characters[ this->m_CharacterIndex ];
    Ogre::SceneNode* home    = entity->getParentSceneNode( );
    const Ogre::Vector3 scale = home->getScale( );

    // Dirección de la mano: del antebrazo al puño, expresada en los ejes del hueso
    Ogre::Skeleton* skeleton = thrower->getSkeleton( );
    skeleton->_updateTransforms( );
    Ogre::Bone* fist    = skeleton->getBone( "Fist.R" );
    Ogre::Bone* forearm = static_cast< Ogre::Bone* >( fist->getParent( ) );

    Ogre::Vector3 along = fist->_getDerivedPosition( ) - forearm->_getDerivedPosition( );
    along.normalise( );
    const Ogre::Vector3 palm = fist->_getDerivedOrientation( ).Inverse( ) * along * kPalmDist;

    // Centro de la caja del objeto, ya con su escala
    const Ogre::Vector3 center = entity->getBoundingBox( ).getCenter( ) * scale;

    home->detachObject( entity );
    Ogre::TagPoint* tag = thrower->attachObjectToBone(
        "Fist.R",
        entity,
        Ogre::Quaternion::IDENTITY,
        palm - center
        );
    tag->setScale( scale );
        this->m_CameraMgr.setFollowedNode( tag );

    this->m_HeldEntity = entity;
    this->m_HeldHome   = home;
}

//----------------------------------------------------------------------------------
// Suelta el proyectil: lo devuelve a su nodo para que lo mueva la física
void Application::
_dropProjectile( )
{
    if( this->m_HeldEntity == nullptr )
        return;

    this->m_Characters[ this->m_CharacterIndex ]->detachObjectFromBone( this->m_HeldEntity );
    this->m_HeldHome->attachObject( this->m_HeldEntity );
    this->m_CameraMgr.setFollowedNode( this->m_HeldHome );
    this->m_HeldEntity = nullptr;
    this->m_HeldHome   = nullptr;
}

//----------------------------------------------------------------------------------
//Actualizar los datos del proyectil
void Application::
_updateResult( )
{
    if( m_Tracking == false ){
        return;
    }

    const bool touching = this->m_Physics.isTouching( this->m_TrackedName );

    if( !this->m_Airborne ){
        this->m_Airborne = !touching;
        return;
    }


    
    if( !touching )
        return;

    const Ogre::Entity* projectile = this->m_SceneMgr->getEntity( this->m_TrackedName );
    const Ogre::Vector3 landing = projectile->getParentSceneNode( )->_getDerivedPosition( );
    const Ogre::Vector3 center = this->m_ThrowerNode->_getDerivedPosition( );
    
    Ogre::Vector3 delta = landing - center;
    delta.y = 0.0f;
    this->m_LastDistance = std::max( 0.0f, delta.length( ) - kCircleRadius );
    this->m_Tracking = false;

    this->m_ResultView.show( this->m_LastDistance );

    std::cout << "[Result] " << this->m_TrackedName
              << " cae en (" << landing.x << ", " << landing.z << ")"
              << " distancia " << this->m_LastDistance << " m" << std::endl;

}

//----------------------------------------------------------------------------------
// Muestra solo al personaje elegido y le pasa animación, cámara y objeto
void Application::
_setCharacter( int index )
{
    Ogre::Entity* held = this->m_HeldEntity;
    this->_dropProjectile( );

    if( this->m_Characters.empty( ) )
        for( const std::string& name : kCharacterNames )
            this->m_Characters.push_back( this->m_SceneMgr->getEntity( name ) );

    for( std::size_t i = 0; i < kCharacterNames.size( ); ++i )
        this->m_SceneMgr->getSceneNode( kCharacterNames[ i ] )->setVisible( static_cast< int >( i ) == index );

    Ogre::SceneNode* old = this->m_ThrowerNode;
    this->m_ThrowerNode  = this->m_SceneMgr->getSceneNode( kCharacterNames[ index ] );
    if( old != nullptr && old != this->m_ThrowerNode )
        this->m_ThrowerNode->setOrientation( old->getOrientation( ) );

    Ogre::Entity* thrower = this->m_Characters[ index ];
    this->m_CharacterIndex = index;
    this->m_Thrower.build( thrower, "", "my_animation", kReleaseTime );

    Ogre::Camera* manCam = this->m_SceneMgr->getCamera( "ManCamera" );
    if( this->m_CameraOwner != nullptr )
        this->m_CameraOwner->detachObjectFromBone( manCam );
    else if( manCam->getParentSceneNode( ) != nullptr )
        manCam->getParentSceneNode( )->detachObject( manCam );

    thrower->attachObjectToBone(
        "Head",
        manCam,
        Ogre::Quaternion( Ogre::Degree( 0 ), Ogre::Vector3::UNIT_X ),
        Ogre::Vector3( 0.0f, 0.1f, 0.1f )
        );
    this->m_CameraOwner = thrower;

    if( held != nullptr )
        this->_holdProjectile( held );
}

//----------------------------------------------------------------------------------
void Application::
_nextCharacter( )
{
    if( this->m_Thrower.isBusy( ) )
        return;
    this->_setCharacter( ( this->m_CharacterIndex + 1 ) % static_cast< int >( kCharacterNames.size( ) ) );
}

//-----------------------------------------------------------------------------------
void Application::
go( )
{
    this->initApp( );
    this->getRoot( )->startRendering( );
    this->closeApp( );
}

//----------------------------------------------------------------------------------
void Application::
_initSceneManager( )
{
    this->TContext::setup( );
    this->addInputListener( this );

    auto* root = this->getRoot( );
    this->m_SceneMgr = root->createSceneManager( "DefaultSceneManager" );
    this->m_SceneMgr->setAmbientLight( Ogre::ColourValue( 0.9, 0.9, 0.9 ) );

    //register our scene with RTSS
    auto* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr( );
    shadergen->addSceneManager( this->m_SceneMgr );

    this->m_SceneMgr->addRenderQueueListener( this->getOverlaySystem( ) );
}

//----------------------------------------------------------------------------------
void Application::
_configureCamera( )
{
    this->m_CameraMgr.build( this->m_SceneMgr, this->getRenderWindow( ), kProjectileNames[ 0 ] );
    this->addInputListener( &this->m_CameraMgr );

    this->m_MeterView.build( );
    this->m_ResultView.build( );
    this->_selectProjectile( 0 );
}

void Application::
_configurePhysics( )
{
    this->m_Physics.build(
        this->m_SceneMgr->getRootSceneNode( ), Ogre::Vector3( 0, -9.81f, 0 )
        );
    this->m_Physics.setSimulating( true );
}

void Application::
_loadScene( )
{
    //this->m_SceneMgr->setDisplaySceneNodes( true );   // en _loadScene, para probar
    auto* root = this->m_SceneMgr->getRootSceneNode( );
    root->loadChildren( "campo.scene");
    root->loadChildren( "bala.scene");
    root->loadChildren( "estadio.scene" );
    root->loadChildren( "pruebas.scene" );
    root->loadChildren( "personaje.scene" );

    this->m_SceneMgr->setSkyBox( true, "Cielo_Azul", 1000.0f );
    this->_setCharacter( 0 ); 
}