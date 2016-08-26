#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    
#ifdef USE_CAMERA
    videoSource.setDesiredFrameRate(30);
    videoSource.initGrabber(640, 480);
#else
    videoSource.load("TOM_Original_480p.mov");
    videoSource.play();
    videoSource.setVolume(0.0f);
#endif
    
    flowSolver.setup(videoSource.getWidth(), videoSource.getHeight(), 0.35, 5, 10, 1, 3, 2.25, false, false);
    flow.allocate(videoSource.getWidth(),videoSource.getHeight(),GL_RGBA);
    flowPixels.allocate(videoSource.getWidth(),videoSource.getHeight(),4);
    flowRight.allocate(videoSource.getWidth(),videoSource.getHeight());
    flowLeft.allocate(videoSource.getWidth(),videoSource.getHeight());
#ifdef _DEBUG
    flowRight.setUseTexture(true);
    flowLeft.setUseTexture(true);
#else
    flowRight.setUseTexture(false);
    flowLeft.setUseTexture(false);
#endif
    
    contourFinderRight.setMinAreaRadius(videoSource.getWidth()*0.05f);
    contourFinderRight.setMaxAreaRadius(videoSource.getWidth());
    contourFinderRight.setThreshold(100);
    // wait for half a frame before forgetting something
    contourFinderRight.getTracker().setPersistence(15);
    // an object can move up to 32 pixels per frame
    contourFinderRight.getTracker().setMaximumDistance(50);
    
    contourFinderLeft.setMinAreaRadius(videoSource.getWidth()*0.05f);
    contourFinderLeft.setMaxAreaRadius(videoSource.getWidth());
    contourFinderLeft.setThreshold(100);
    // wait for half a frame before forgetting something
    contourFinderLeft.getTracker().setPersistence(15);
    // an object can move up to 32 pixels per frame
    contourFinderLeft.getTracker().setMaximumDistance(50);
    
    ofEnableAlphaBlending();
    ofSetBackgroundAuto(true);
    ofBackground(0);
    
    ofSetWindowShape(videoSource.getWidth()*2, videoSource.getHeight()*2);
}

//--------------------------------------------------------------
void ofApp::update() {
    videoSource.update();
    if(videoSource.isFrameNew()) {
        flowSolver.update(videoSource);
        
        flow.begin();
        ofClear(0,0);
        flowSolver.drawColored(videoSource.getWidth(), videoSource.getHeight(), 1, 1);
        flow.end();
        
        flow.readToPixels(flowPixels);
        //flow.readToPixels(flowImage.getPixels());
        flowRight.setFromPixels(flowPixels.getChannel(0));
        flowLeft.setFromPixels(flowPixels.getChannel(2));
        
        //flowRight.threshold(100);
        //flowLeft.threshold(100);
        
        flowRight.dilate();
        flowRight.erode();
        
        flowLeft.dilate();
        flowLeft.erode();
        
        contourFinderRight.findContours(flowRight);
        contourFinderLeft.findContours(flowLeft);
#ifdef _DEBUG
        flowRight.updateTexture();
        flowLeft.updateTexture();
#endif
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
#ifdef _DEBUG
    ofEnableAlphaBlending();
    
    ofSetColor(255, 255, 255);
    videoSource.draw(0, 0,videoSource.getWidth(), videoSource.getHeight());
    
    ofPushMatrix();
    ofTranslate(videoSource.getWidth(),0);
    videoSource.draw(0, 0);
    flow.draw(0,0);
    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(0,videoSource.getHeight());
    flowRight.draw(0,0);
    contourFinderRight.draw();
    ofxCv::RectTracker& trackerRight = contourFinderRight.getTracker();
    for(int i = 0; i < contourFinderRight.size(); i++) {
        ofPoint center = ofxCv::toOf(contourFinderRight.getCenter(i));
        ofPushMatrix();
        ofTranslate(center.x, center.y);
        int label = contourFinderRight.getLabel(i);
        string msg = ofToString(label) + ":" + ofToString(trackerRight.getAge(label));
        ofDrawBitmapStringHighlight(msg, 0, 0);
        ofVec2f velocity = ofxCv::toOf(contourFinderRight.getVelocity(i));
        ofScale(5, 5);
        ofPushStyle();
        ofSetColor(0,255,255);
        ofSetLineWidth(5.0f);
        ofDrawLine(0, 0, velocity.x, velocity.y);
        ofPopStyle();
        ofPopMatrix();
    }
    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(videoSource.getWidth(),videoSource.getHeight());
    flowLeft.draw(0,0);
    contourFinderLeft.draw();
    ofxCv::RectTracker& trackerLeft = contourFinderLeft.getTracker();
    for(int i = 0; i < contourFinderLeft.size(); i++) {
        ofPoint center = ofxCv::toOf(contourFinderLeft.getCenter(i));
        ofPushMatrix();
        ofTranslate(center.x, center.y);
        int label = contourFinderLeft.getLabel(i);
        string msg = ofToString(label) + ":" + ofToString(trackerLeft.getAge(label));
        ofDrawBitmapStringHighlight(msg, 0, 0);
        ofVec2f velocity = ofxCv::toOf(contourFinderLeft.getVelocity(i));
        ofScale(5, 5);
        ofPushStyle();
        ofSetColor(255,255,0);
        ofSetLineWidth(5.0f);
        ofDrawLine(0, 0, velocity.x, velocity.y);
        ofPopStyle();
        ofPopMatrix();
    }
    ofPopMatrix();
#endif
    
    stringstream m;
    m << "fps " << ofGetFrameRate() << endl
    << "pyramid scale: " << flowSolver.getPyramidScale() << " p/P" << endl
    << "pyramid levels: " << flowSolver.getPyramidLevels() << " l/L" << endl
    << "averaging window size: " << flowSolver.getWindowSize() << " w/W" << endl
    << "iterations per level: " << flowSolver.getIterationsPerLevel() << " i/I" << endl
    << "expansion area: " << flowSolver.getExpansionArea() << " a/A" << endl
    << "expansion sigma: " << flowSolver.getExpansionSigma() << " s/S" << endl
    << "flow feedback: " << flowSolver.getFlowFeedback() << " f/F" << endl
    << "gaussian filtering: " << flowSolver.getGaussianFiltering() << " g/G";
    
    ofDrawBitmapString(m.str(), 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if(key == 'p') { flowSolver.setPyramidScale(ofClamp(flowSolver.getPyramidScale() - 0.01,0.0,1.0)); }
    else if(key == 'P') { flowSolver.setPyramidScale(ofClamp(flowSolver.getPyramidScale() + 0.01,0.0,1.0)); }
    else if(key == 'l') { flowSolver.setPyramidLevels(MAX(flowSolver.getPyramidLevels() - 1,1)); }
    else if(key == 'L') { flowSolver.setPyramidLevels(flowSolver.getPyramidLevels() + 1); }
    else if(key == 'w') { flowSolver.setWindowSize(MAX(flowSolver.getWindowSize() - 1,1)); }
    else if(key == 'W') { flowSolver.setWindowSize(flowSolver.getWindowSize() + 1); }
    else if(key == 'i') { flowSolver.setIterationsPerLevel(MAX(flowSolver.getIterationsPerLevel() - 1,1)); }
    else if(key == 'I') { flowSolver.setIterationsPerLevel(flowSolver.getIterationsPerLevel() + 1); }
    else if(key == 'a') { flowSolver.setExpansionArea(MAX(flowSolver.getExpansionArea() - 2,1)); }
    else if(key == 'A') { flowSolver.setExpansionArea(flowSolver.getExpansionArea() + 2); }
    else if(key == 's') { flowSolver.setExpansionSigma(ofClamp(flowSolver.getExpansionSigma() - 0.01,0.0,10.0)); }
    else if(key == 'S') { flowSolver.setExpansionSigma(ofClamp(flowSolver.getExpansionSigma() + 0.01,0.0,10.0)); }
    else if(key == 'f') { flowSolver.setFlowFeedback(false); }
    else if(key == 'F') { flowSolver.setFlowFeedback(true); }
    else if(key == 'g') { flowSolver.setGaussianFiltering(false); }
    else if(key == 'G') { flowSolver.setGaussianFiltering(true); }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
