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
    
    backgroundMoving=1.0f;
    vehiclesMoving=1.0f;
    
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
        
        flowRight.threshold(100);
        flowLeft.threshold(100);
        
        flowRight.dilate();
        flowRight.erode();
        
        flowLeft.dilate();
        flowLeft.erode();
        
        contourFinderRight.findContours(flowRight,
                                        videoSource.getWidth()*videoSource.getHeight()*0.05f,
                                        videoSource.getWidth()*videoSource.getHeight()*0.90f,
                                        10,
                                        false);
        contourFinderLeft.findContours(flowLeft,
                                       videoSource.getWidth()*videoSource.getHeight()*0.05f,
                                       videoSource.getWidth()*videoSource.getHeight()*0.90f,
                                       10,
                                       false);
        
        vector<float> blobAreas;
        float accumulatedArea = 0.0f;
        for(int i = 0; i < contourFinderRight.nBlobs; i++){
            blobAreas.push_back(contourFinderRight.blobs[i].area);
            accumulatedArea += blobAreas.back();
        }
        for(int i = 0; i < contourFinderLeft.nBlobs; i++){
            blobAreas.push_back(contourFinderLeft.blobs[i].area);
            accumulatedArea += blobAreas.back();
        }
        
        std::sort(blobAreas.begin(),blobAreas.end());
        
        //this means the biggest blob found is most probably also the background
        if(blobAreas.size() && blobAreas.front() > (videoSource.getWidth()*videoSource.getHeight()*0.45f)){
            backgroundMoving = 1.0f;
            accumulatedArea -= blobAreas.front();
        }
        else{
            backgroundMoving -= 0.1f;
            if(backgroundMoving<=0.0f)
                backgroundMoving=0.0f;
        }
        
        if(accumulatedArea > (videoSource.getWidth()*videoSource.getHeight()*0.1f)){
            vehiclesMoving=1.0f;
        }
        else{
            vehiclesMoving -= 0.1f;
            if(vehiclesMoving<=0.0f)
                vehiclesMoving=0.0f;
        }
        
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
    for(int i = 0; i < contourFinderRight.nBlobs; i++) {
        ofPoint center = contourFinderRight.blobs[i].boundingRect.getCenter();
        ofPushMatrix();
        ofTranslate(center.x, center.y);
        ofDrawBitmapStringHighlight(ofToString(i), 0, 0);
        ofPopMatrix();
    }
    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(videoSource.getWidth(),videoSource.getHeight());
    flowLeft.draw(0,0);
    contourFinderLeft.draw();
    for(int i = 0; i < contourFinderLeft.nBlobs; i++) {
        ofPoint center = contourFinderLeft.blobs[i].boundingRect.getCenter();
        ofPushMatrix();
        ofTranslate(center.x, center.y);
        ofDrawBitmapStringHighlight(ofToString(i), 0, 0);
        ofPopMatrix();
    }
    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(ofGetWidth()*0.5,ofGetHeight()*0.5);
    ofScale(50,50);
    ofPushStyle();
    ofSetRectMode(ofRectMode::OF_RECTMODE_CENTER);
    ofSetColor(0);
    ofDrawRectangle(0,0,1.1,4.1);
    ofSetColor(255);
    ofNoFill();
    ofDrawCircle(0,-1.5,0.5);
    ofDrawCircle(0,0,0.5);
    ofDrawCircle(0,1.5,0.5);
    ofFill();
    if(vehiclesMoving){
        ofSetColor(255,0,0);
        ofDrawCircle(0,-1.5,0.5);
    }
    if(backgroundMoving){
        ofSetColor(255,255,0);
        ofDrawCircle(0,0,0.5);
    }
    if(!vehiclesMoving && !backgroundMoving){
        ofSetColor(0,255,0);
        ofDrawCircle(0,1.5,0.5);
    }
    ofPopStyle();
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
#ifdef _DEBUG
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
#endif
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
