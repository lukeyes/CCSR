#ifdef __cplusplus
extern "C" {
#endif

// ROI is region of interest: square box center-image that is assumed to be contained inside an object
// to be analyzed for color content. May need to tweak this based on object size and CCSR manouvering
// precision.
#define ROI_HEIGHT 50
#define ROI_WIDTH  50
#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480
#define MIN_TRACKED_OBJECT_VOLUME 50000    // Objects smaller (or further away) than this will not be tracked

#define MAX_HUE         179
#define MAX_SATURATION  255
#define MAX_VALUE       255

// Define ranges around an HSV color value that classify a single color.
#define HUE_WINDOW        20
#define SATURATION_WINDOW 40
#define VALUE_WINDOW      20

// We test CCSR with colored balls of fixed size. This number is the volume of the tracked object as seen by *visual
// when is it close enough to be picked up.
#define TEST_OBJECT_1_VOLUME 2000000   // object fitting in CCSR's grabber
#define TEST_OBJECT_2_VOLUME 2000000   // Blue box

// If object is +- 10 pixels from X,Y center, we can pick it up with grabber
#define OBJECT_PICKUP_WINDOW_X 10 
#define OBJECT_PICKUP_WINDOW_Y 10
#define OBJECT_PICKUP_OFFSET_Y 130  // Use this offset to position CCSR on Y-axis such that arm can grab target object


void *visual ();
void setTargetColorRange(int iLowH, int iHighH, int iLowS, int iHighS, int iLowV, int iHighV);
void setTargetColorVolume(int vol);

#ifdef __cplusplus
}
#endif

