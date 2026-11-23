#ifndef __Application__h__
#define __Application__h__

#include <OgreApplicationContext.h>
#include "PhysicsModel.h"
#include "CameraManager.h"
#include "ShootController.h"
#include "ShootMeterView.h"

namespace Ogre      { class Entity; }

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
    virtual void frameRendered( const Ogre::FrameEvent& evt ) override;
    virtual bool keyPressed( const OgreBites::KeyboardEvent& evt ) override;
    virtual bool keyReleased( const OgreBites::KeyboardEvent& evt ) override;
    virtual bool mouseWheelRolled( const OgreBites::MouseWheelEvent& evt ) override;

    // Main method
    virtual void go( );

protected:
    virtual void _initSceneManager( );
    virtual void _configureCamera( );
    virtual void _loadScene( );
    virtual void _configurePhysics( );
    void _shootBall( float speed );
    void _selectProjectile( std::size_t index );
    void _cycleProjectile( int step );

protected: 
    std::string m_ResourcesFile;
    
    Ogre::SceneManager* m_SceneMgr = nullptr;
    CameraManager m_CameraMgr;
    ShootController m_Shot; 
    ShootMeterView  m_MeterView;
    PhysicsModel m_Physics;
    std::size_t m_CurrentProjectile = 0;
    bool m_ObjectKeyHeld = false;
};


#endif 