#ifndef __Application__h__
#define __Application__h__

#include <OgreApplicationContext.h>

namespace Ogre      { class Entity; }
namespace OgreBites { class CameraMan; }

class Application
    : public OgreBites::ApplicationContext,
      public OgreBites::InputListener
{
public:
    using TContext  = OgreBites::ApplicationContext;
    using TListener = OgreBites::InputListener;

public:
    Application( const std::string& app_name, const std::string& main_dir );
    virtual ~Application( ) override = default; 
    
    // Configuration
    virtual void loadResources( ) override;
    virtual void setup( ) override; 

    // Interactive methods
    virtual bool frameStarted( const Ogre::FrameEvent& evt ) override;
    virtual bool keyPressed( const OgreBites::KeyboardEvent& evt ) override;

    // Main method
    virtual void go( );

protected:
    virtual void _initSceneManager( );
    virtual void _configureCamera( );
    virtual void _loadScene( );

protected: 
    std::string m_ResourcesFile;
    
    Ogre::SceneManager* m_SceneMgr;
    OgreBites::CameraMan* m_CamMan;
};

#endif 