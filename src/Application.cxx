# include <iostream>
# include "Application.h"

# include <filesystem>
# include <vector>

# include <OgreOverlaySystem.h>
# include <Ogre.h>
# include <OgreCameraMan.h>

namespace
{
    const std::vector< std::string > kProjectileNames = { "yellow_ball", "rubber_duck", "banana", "cerveza" };

    const float kLaunchSpeed            = 12.0f;
    const float kLaunchUpwardRatio      = 0.84f;
    const float kLaunchForwardOffset    = 0.5f;
    const float kMinGlobalCameraY       = 0.5f;   
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
    return( this->TContext::frameStarted( evt ) );
}

//----------------------------------------------------------------------------------
void Application::
frameRendered( const Ogre::FrameEvent& evt )
{
    this->m_Physics.update( evt.timeSinceLastFrame );
    this->m_CameraMgr.update( );

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
    if( evt.keysym.sym == 'c' ) this->m_CameraMgr.setCycling( true );

    if( evt.keysym.sym == ' ' && !evt.repeat && this->m_CameraMgr.isActive( CameraManager::Man ) )
    {
        const float power = this->m_Shot.getPower( );
        std::cout<< "[Shoot] fuerza " << power << std::endl;
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

    const Ogre::Vector3 origin
        = cam->getDerivedPosition( ) + cam->getDerivedDirection( ) * kLaunchForwardOffset;

    this->m_Physics.launch( kProjectileNames[ this->m_CurrentProjectile ], origin, direction * speed );
}

//----------------------------------------------------------------------------------
// Deja activo solo el proyectil actual y oculta lo demas. SObre el actual se hace la física
void Application::
_selectProjectile( std::size_t index )
{
    for( std::size_t i = 0; i < kProjectileNames.size( ); ++i )
    {
        const bool active = ( i == index );
        this->m_Physics.setBodyEnabled( kProjectileNames[ i ], active );
        this->m_SceneMgr->getEntity( kProjectileNames[ i ] )->setVisible( active );
    }

    this->m_CurrentProjectile = index;
    Ogre::Entity* entity = this->m_SceneMgr->getEntity( kProjectileNames[ index ] );
    this->m_CameraMgr.setFollowedNode( entity->getParentSceneNode( ) );
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
}