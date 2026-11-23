#ifndef __CameraManager_h__
#define __CameraManager_h__

#include <string>
#include <vector>
#include <OgreInput.h>

namespace Ogre          { class Camera; class Viewport; class SceneManager; 
                          class SceneNode; class RenderWindow; }
namespace OgreBites     { class CameraMan; }

class CameraManager
    : public OgreBites::InputListener
{
public: 
    using Self = CameraManager;

    enum CameraId { Ball = 0, Fixed = 1, Global = 2, Man = 3, Count };

public: 
    CameraManager( ) = default; 
    virtual ~CameraManager( ) override; 

    void build( Ogre::SceneManager* sceneMgr, Ogre::RenderWindow* window, const std::string& followedNodeName );
    void select ( CameraId id );
    void selectNext( );
    void selectPrevious( );
    bool isActive( CameraId id ) const; 

    Ogre::Camera* getActiveCamera( ) const;

    void update( );

    virtual bool mouseMoved( const OgreBites::MouseMotionEvent& evt ) override;
    virtual bool mousePressed( const OgreBites::MouseButtonEvent& evt ) override;
    virtual bool mouseReleased( const OgreBites::MouseButtonEvent& evt ) override; 
    
    void setCycling( bool enabled );
    void setFollowedNode( Ogre::SceneNode* node );
    void setWheelBlocked( bool blocked );

    virtual bool mouseWheelRolled( const OgreBites::MouseWheelEvent& evt ) override;

private: 
    Ogre::Camera* _requireCamera( Ogre::SceneManager* sceneMgr, const std::string& name );
    bool _isGlobalActive( ) const;

private: 
    Ogre::Viewport*                 m_Viewport = nullptr; 
    std::vector< Ogre::Camera* >    m_Cameras; 
    std::size_t                     m_Active = Global; 
    Ogre::SceneNode*                m_BallCameraNode = nullptr;
    Ogre::SceneNode*                m_FollowedNode = nullptr;
    OgreBites::CameraMan*           m_CamMan = nullptr;
    bool m_WheelBlocked = false;
    bool m_Cycling = false;
};

#endif