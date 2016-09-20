#include "ofApp.h"

#define BACKGROUND_TIMEOUT 0.5f
#define VEHICLE_TIMEOUT 1.0f

#define STOP_PERIOD 2.0f
#define START_PERIOD 1.0f

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
    
    stopSound.load("stop.mp3");
    startSound.load("start.mp3");
    
    flowSolver.setup(videoSource.getWidth(), videoSource.getHeight());
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
    
    backgroundMoving=BACKGROUND_TIMEOUT;
    vehiclesMoving=VEHICLE_TIMEOUT;
    
    stopSoundPlay=true;
    stopSoundTimer=0.0f;
    startSoundTimer=START_PERIOD;
    
    ofEnableAlphaBlending();
    ofSetBackgroundAuto(true);
    ofBackground(0);
    
    ofSetWindowShape(videoSource.getWidth()*2, videoSource.getHeight()*2);
    
    time = ofGetElapsedTimef();
}

//--------------------------------------------------------------
void ofApp::update() {
    float t = ofGetElapsedTimef();
    float dt = t - time;
    time = t;
    
    videoSource.update();
    if(videoSource.isFrameNew()) {
        flowSolver.update(videoSource.getTexture());
        
        flow.begin();
        ofClear(0,0);
        flowSolver.drawFlowGrid(0,0);
        flow.end();
        
        flow.readToPixels(flowPixels);
        //flow.readToPixels(flowImage.getPixels());
        flowRight.setFromPixels(flowPixels.getChannel(0));
        flowLeft.setFromPixels(flowPixels.getChannel(1));
        
        flowRight.threshold(100);
        flowLeft.threshold(100);
        
        //flowRight.dilate();
        //flowRight.erode();
        
        //flowLeft.dilate();
        //flowLeft.erode();
        
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
            backgroundMoving = BACKGROUND_TIMEOUT;
            accumulatedArea -= blobAreas.front();
        }
        else{
            backgroundMoving -= dt;
            if(backgroundMoving<=0.0f)
                backgroundMoving=0.0f;
        }
        
        if(accumulatedArea > (videoSource.getWidth()*videoSource.getHeight()*0.075f)){
            vehiclesMoving = VEHICLE_TIMEOUT;
        }
        else{
            vehiclesMoving -= dt;
            if(vehiclesMoving<=0.0f)
                vehiclesMoving=0.0f;
        }
        
#ifdef _DEBUG
        flowRight.updateTexture();
        flowLeft.updateTexture();
#endif
    }
    
    if(backgroundMoving>0 || vehiclesMoving>0){
        if(!stopSoundPlay)
            stopSoundTimer=0.0f;
        stopSoundPlay=true;
    }
    else{
        if(stopSoundPlay)
            startSoundTimer = START_PERIOD;
        stopSoundPlay=false;
    }
    if(stopSoundPlay){
        stopSoundTimer-=dt;
        if(stopSoundTimer<=0.0f){
            stopSound.play();
            stopSoundTimer = STOP_PERIOD;
        }
    }
    else{
        startSoundTimer-=dt;
        if(startSoundTimer<=0.0f){
            startSound.play();
            startSoundTimer = START_PERIOD;
        }
    }
    
    ofSoundUpdate();
}

//--------------------------------------------------------------
void ofApp::draw() {
#ifdef _DEBUG
    ofEnableAlphaBlending();
    
    ofSetColor(255, 255, 255);
    videoSource.draw(0, 0,videoSource.getWidth(), videoSource.getHeight());
    
    ofPushMatrix();
    ofTranslate(videoSource.getWidth(),0);
    //videoSource.draw(0, 0);
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
    m << "fps " << ofGetFrameRate() << endl;
    
    ofDrawBitmapString(m.str(), 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
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
