#ifndef __Application__h__
#define __Application__h__

#include <OgreApplicationContext.h>
#include <vector>

#include "PhysicsModel.h"
#include "CameraManager.h"
#include "ShootController.h"
#include "ShootMeterView.h"
#include "ThrowerAnimator.h"
#include "ResultView.h"

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
    virtual void    _initSceneManager( );
    virtual void    _configureCamera( );
    virtual void    _loadScene( );
    virtual void    _configurePhysics( );
    void            _shootBall( float speed );
    void            _selectProjectile( std::size_t index );
    void            _cycleProjectile( int step );
    void            _holdProjectile( Ogre::Entity* entity );
    void            _dropProjectile( );
    void            _updateResult( );
    void            _setCharacter( int index );
    void            _nextCharacter( );

protected: 
    std::string             m_ResourcesFile;
    
    Ogre::SceneManager*     m_SceneMgr = nullptr;
    CameraManager           m_CameraMgr;
    ShootController         m_Shot; 
    ShootMeterView          m_MeterView;
    PhysicsModel            m_Physics;
    std::size_t             m_CurrentProjectile = 0;
    bool                    m_ObjectKeyHeld = false;
    ThrowerAnimator         m_Thrower;
    int                     m_CharacterIndex = 1;
    std::vector< Ogre::Entity* > m_Characters;
    Ogre::Entity*           m_CameraOwner = nullptr;
    Ogre::SceneNode*        m_ThrowerNode = nullptr;
    Ogre::Entity*           m_HeldEntity = nullptr;
    Ogre::SceneNode*        m_HeldHome   = nullptr;
    bool                    m_Tracking     = false;
    bool                    m_Airborne     = false;
    std::string             m_TrackedName;
    float                   m_LastDistance = 0.0f;
    ResultView              m_ResultView;
};


#endif 