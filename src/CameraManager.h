#ifndef __CameraManager_h__
#define __CameraManager_h__

#include <string>
#include <vector>
#include <OgreInput.h>
#include <OgreVector.h>

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
    void setFollowedNode( Ogre::Node* node );
    void setWheelBlocked( bool blocked );

    virtual bool mouseWheelRolled( const OgreBites::MouseWheelEvent& evt ) override;

private: 
    Ogre::Camera* _requireCamera( Ogre::SceneManager* sceneMgr, const std::string& name );
    bool _isGlobalActive( ) const;
    void _keepGlobalCameraInWorld( );
    void _buildForbiddenZone( Ogre::SceneManager* sceneMgr );
    bool _isInsideForbiddenZone( const Ogre::Vector3& point ) const;
    void _keepGlobalCameraOutOfZone( );

private: 
    Ogre::Viewport*                 m_Viewport = nullptr; 
    std::vector< Ogre::Camera* >    m_Cameras; 
    std::size_t                     m_Active = Global; 
    Ogre::SceneNode*                m_BallCameraNode = nullptr;
    Ogre::Node*                     m_FollowedNode = nullptr;
    OgreBites::CameraMan*           m_CamMan = nullptr;
    bool m_WheelBlocked = false;
    bool m_Cycling = false;
    std::vector< Ogre::Vector3 > m_ZoneTriangles; 
    Ogre::Vector3                m_LastGoodPosition;
};

#endif