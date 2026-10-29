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
            res_mgr->addResourceLocation( fIt->second, fIt->first, sIt->first );

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
    this->_configureCamera( );
}

//----------------------------------------------------------------------------------
bool Application::
frameStarted( const Ogre::FrameEvent& evt )
{
    return( this->TContext::frameStarted( evt ) );
}

//----------------------------------------------------------------------------------
bool Application::
keyPressed( const OgreBites::KeyboardEvent& evt )
{
    if( evt.keysym.sym == OgreBites::SDLK_ESCAPE){
        this->getRoot( )->queueEndRendering( );
    }
    return( true );
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
    auto* cam = this->m_SceneMgr->getCameras( ).begin( )->second;
    cam->setAutoAspectRatio( true );

    auto* camman = new OgreBites::CameraMan( cam-> getParentSceneNode( ) );
    camman->setStyle( OgreBites::CS_ORBIT );
    this->addInputListener( camman );
    this->getRenderWindow( )->addViewport( cam );
}

void Application::
_loadScene( )
{
    auto* root = this->m_SceneMgr->getRootSceneNode( );
    root->loadChildren( "athletics.scene");
}
