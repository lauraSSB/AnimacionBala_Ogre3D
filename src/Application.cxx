# include <iostream>
# include "Application.h"

# include <filesystem>

# include <Ogre.h>
# include <OgreCameraMan.h>

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
    return ( true );
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
}

//----------------------------------------------------------------------------------
void Application::
_configureCamera( )
{
    this->m_CameraMgr.build( this->m_SceneMgr, this->getRenderWindow( ), "yellow_ball" );
    this->addInputListener( &this->m_CameraMgr );
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
}