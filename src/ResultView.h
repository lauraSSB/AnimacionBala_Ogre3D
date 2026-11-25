#ifndef __ResultView__h__
#define __ResultView__h__

namespace Ogre { class Overlay; class OverlayElement; }

class ResultView
{
public:
    void build( );

    void show( float distance );

    void hide( );

private:
    Ogre::Overlay*        m_Overlay = nullptr;
    Ogre::OverlayElement* m_Text    = nullptr;
};

#endif