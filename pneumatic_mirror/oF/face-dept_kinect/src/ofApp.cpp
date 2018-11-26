#include "ofApp.h"

int xEyePos;
int yEyePos;
ofSerial serial;

//to be sent to arduino
    int blowerOn = 65; // 65 for off, 66 for on (for controlling blower motor power)
    int motorMove = 66; //65 for down, 66 for hold, 67 for up
    int servoPos = 59; //from 81 to 139

unsigned char arduinoBytes[4] = {65, 66, 59, 33};

int servoMin = 34; //based on valves for airflow
int servoMax = 122;
int faceTimeout= 2000; // to allow a short period of dropped face detection before turning off blower in milliseconds
int minFaceDist = 1100;
int maxFaceDist = 3000; //≈3500 indoors at night with imperfect lighting
float zAccel;
int alignmentThreshold = 7;
int slowDownThreshold = 20;
int calculatedYpos;
float vOffset = 335; //millimeters
float vFOV = 0.4066617; //radians for 1/2 vertical FOV of depth cam
int intraOcularDist = 0;
int faceDepth = minFaceDist;
float faceAngle;
float angleDiscrepancy;
int faceDepthPrev;

using namespace ofxCv;

//--------------------------------------------------------------
void ofApp::setup() {
//    serial.listDevices();
//    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    int baud = 9600;
    serial.setup("/dev/cu.usbmodem1411", baud); // mac osx example
    kinect.setRegistration(true);
    kinect.init();
    kinect.open();
    
    grayImage.allocate(kinect.width, kinect.height);
    colorImg.allocate(kinect.width, kinect.height);
    xEyePos = kinect.width/2;
    yEyePos = kinect.height/2;
    ofSetFrameRate(7);
    tracker.setup();
    tracker.setRescale(1);
    //kinect.setCameraTiltAngle(0);
}

//--------------------------------------------------------------
void ofApp::update() {
    
    ofBackground(100, 100, 100);
    
    kinect.update();

    if(kinect.isFrameNew()) {
        grayImage.setFromPixels(kinect.getDepthPixels());
        tracker.update(toCv(kinect));
    }

}

//--------------------------------------------------------------
void ofApp::draw() {
    
    stringstream reportStream;
    zAccel = kinect.getMksAccel().z;
    
        ofSetColor(255);
    kinect.draw(0, 0, kinect.width, kinect.height);
    
    ofPolyline leftEye = tracker.getImageFeature(ofxFaceTracker::LEFT_EYE);
    ofPolyline rightEye = tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE);
    ofSetColor(ofColor::red);
    if (leftEye.getCentroid2D().x){
        ofResetElapsedTimeCounter(); //replace later?
        blowerOn = 66;
        faceDepth = 0;
        xEyePos = (leftEye.getCentroid2D().x + rightEye.getCentroid2D().x) / 2;
        yEyePos = (leftEye.getCentroid2D().y + rightEye.getCentroid2D().y) / 2;
        intraOcularDist = pow(pow(abs(leftEye.getCentroid2D().x -         rightEye.getCentroid2D().x), 2) + pow(abs(leftEye.getCentroid2D().y - rightEye.getCentroid2D().y), 2), .5);
        for (int x = 0; x < intraOcularDist; x++){
            for (int y = 0; y < intraOcularDist; y++){
                faceDepth += kinect.getDistanceAt(xEyePos - intraOcularDist/2 + x, yEyePos - intraOcularDist/2 + y);
            }
        }
        faceDepth = faceDepth / pow(intraOcularDist, 2);
        if (faceDepth < minFaceDist){
            faceDepth = minFaceDist;
        }
        if (faceDepth > maxFaceDist){
            faceDepth = maxFaceDist;
        }
        faceAngle = asin(vOffset / faceDepth);
        angleDiscrepancy = faceAngle/vFOV;
        calculatedYpos = angleDiscrepancy * kinect.height/2 + kinect.height/2;
        servoPos = ofMap(faceDepth, minFaceDist, maxFaceDist, servoMin, servoMax);
        
        
        if (zAccel > -4.5 && zAccel < 4.2){
            
            //        if (abs(calculatedYpos - yEyePos) > slowDownThreshold  && leftEye.getCentroid2D().x){
            if ((calculatedYpos - yEyePos) < -alignmentThreshold){
                motorMove = 65;
            }
            else if ((calculatedYpos - yEyePos) > alignmentThreshold){
                motorMove = 67;
            }
            else if (abs(calculatedYpos - yEyePos) < alignmentThreshold){
                motorMove = 66;
                ofSetColor(ofColor::green);
            }
        }
        else if (zAccel <= -4.5){
            motorMove = 66;
        }
        else if (zAccel >= 1.2){
            motorMove = 66;
        }
        
        
    }
    else{
        ofSetColor(ofColor::blue);
        if (ofGetElapsedTimeMillis() > faceTimeout){ //replace later?
        blowerOn = 65;
        motorMove = 66;
        if (zAccel <= -3.5){
            motorMove = 67;
        }
        else if (zAccel >= .75){
            motorMove = 65;
        }
        
        }
    }
//    ofResetElapsedTimeCounter();
//    if (ofGetElapsedTimeMillis() > faceTimeout){
    arduinoBytes[0] = blowerOn;
    arduinoBytes[1] = motorMove;
    arduinoBytes[2] = servoPos;
//    arduinoBytes[0] = 66;
//    arduinoBytes[1] = 66;
//    arduinoBytes[2] = 34; //34-84
//    }
    serial.writeBytes(&arduinoBytes[0], 4);
    

    ofDrawCircle(xEyePos, yEyePos, intraOcularDist/2);
    ofDrawCircle(0, calculatedYpos, intraOcularDist/2);
    
    ofSetColor(220);
    //faceDepth;
    reportStream << faceDepth;//arduinoBytes[0] << arduinoBytes[1] << arduinoBytes[2] << arduinoBytes[3];
    ofDrawBitmapString(reportStream.str(), 40, 20);
    


}

//--------------------------------------------------------------
void ofApp::exit() {
    kinect.close();

}

//--------------------------------------------------------------
void ofApp::keyPressed (int key) {
    switch (key) {
    }
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
    
}
