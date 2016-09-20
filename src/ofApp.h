#pragma once

#include "ofMain.h"
#include "ofxOpticalFlowFarneback.h"

//#define USE_CAMERA
#define _DEBUG

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
#ifdef USE_CAMERA
    ofVideoGrabber videoSource;
#else
    ofVideoPlayer videoSource;
#endif
    
    ofxCvColorImage originalImage,processingImage;
    float width,height;
    ofxOpticalFlowFarneback flowSolver;
    ofFbo flow;
    ofPixels flowPixels;
    ofxCvGrayscaleImage flowRight,flowLeft;
    
    ofxCvContourFinder contourFinderRight,contourFinderLeft;
    
    float backgroundMoving,vehiclesMoving;
    
    ofSoundPlayer stopSound,startSound;
    float stopSoundTimer,startSoundTimer;
    bool stopSoundPlay;
    
    float time;
};
