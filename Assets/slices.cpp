#define Startup1 0
#define Startup2 1
#define Startup3 2
#define trans1 3
#define trans2 4
#define trans3 5
#define trans4 6
#define trans5 7
#define trans6 8
#define trans7 9
#define trans8 10
#define tremplin1 11
#define tremplin2 12
#define tremplin3 13
#define tunnel1 14
#define tunnel2 15
#define tunnel3 16
shapeSegment_t Startup1Segs[] = {
{0.00000f, 0.00000f, -2.00000f, 0.00000f, 0x80}
,{-2.00000f, 0.00000f, -3.00000f, -0.60000f, 0x80}
,{-3.00000f, -0.60000f, -7.00000f, -0.60000f, 0x80}
,{-7.00000f, -0.60000f, -8.00000f, 0.00000f, 0x80}
,{-8.00000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 3.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, 0.00000f, 0x80}
,{-15.00000f, 0.00000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
};

shapeSegment_t Startup1SegsTransparent[] = {
{-2.00630f, -0.19560f, -7.99290f, -0.19560f, 0x60}
};

shapeSegment_t Startup1SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0x80}
};

shapeSegment_t Startup1SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0x80}
};

shapeSegment_t Startup2Segs[] = {
{0.00000f, 0.00000f, -2.00000f, 0.00000f, 0x80}
,{-2.00000f, 0.00000f, -3.00000f, 0.00000f, 0x80}
,{-3.00000f, 0.00000f, -7.00000f, 0.00000f, 0x80}
,{-7.00000f, 0.00000f, -8.00000f, 0.00000f, 0x80}
,{-8.00000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 3.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, 0.00000f, 0x80}
,{-15.00000f, 0.00000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
};

shapeSegment_t Startup2SegsTransparent[] = {
{-2.00630f, -0.19560f, -7.99290f, -0.19560f, 0x60}
};

shapeSegment_t Startup2SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0x80}
};

shapeSegment_t Startup2SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0x80}
};

shapeSegment_t Startup3Segs[] = {
{0.00000f, 0.00000f, -1.98170f, 0.00000f, 0x80}
,{-1.98170f, 0.00000f, -3.98960f, 0.00000f, 0x80}
,{-3.98960f, 0.00000f, -5.99740f, 0.00000f, 0x80}
,{-5.99740f, 0.00000f, -7.97180f, 0.00000f, 0x80}
,{-7.97180f, 0.00000f, -10.01310f, 0.00000f, 0x80}
,{-10.01310f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -12.60590f, 1.81770f, 0x80}
,{-12.60590f, 1.81770f, -13.00000f, 3.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, 0.00000f, 0x80}
,{-15.00000f, 0.00000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
};

shapeSegment_t Startup3SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0x80}
};

shapeSegment_t Startup3SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0x80}
};

shapeSegment_t trans1Segs[] = {
{-0.00000f, -0.50000f, -0.00000f, 0.00000f, 0x80}
,{-0.00000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 3.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, 0.00000f, 0x80}
,{-15.00000f, 0.00000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, -0.00000f, -2.00000f, 0x80}
,{-0.00000f, -2.00000f, -0.00000f, -1.50000f, 0x80}
,{-0.00000f, -1.50000f, -0.00000f, -0.50000f, 0x80}
};

shapeSegment_t trans1SegsTransparent[] = {
{-0.00000f, -0.50000f, -2.00000f, -0.50000f, 0x60}
};

shapeSegment_t trans1SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0xff}
};

shapeSegment_t trans1SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0xff}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0xff}
};

shapeSegment_t trans2Segs[] = {
{-2.00000f, -0.50000f, -3.00000f, 0.00000f, 0x80}
,{-3.00000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 3.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, 0.00000f, 0x80}
,{-15.00000f, 0.00000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, -3.00000f, -2.00000f, 0x80}
,{-3.00000f, -2.00000f, -2.00000f, -1.50000f, 0x80}
,{-2.00000f, -1.50000f, -2.00000f, -0.50000f, 0x80}
};

shapeSegment_t trans2SegsTransparent[] = {
{0.00000f, -0.50000f, -2.00000f, -0.50000f, 0x60}
};

shapeSegment_t trans2SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0xff}
};

shapeSegment_t trans2SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0xff}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0xff}
};

shapeSegment_t trans3Segs[] = {
{-6.00000f, -0.50000f, -7.00000f, 0.00000f, 0x80}
,{-7.00000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 0.20000f, 0x80}
,{-13.00000f, 0.20000f, -14.00000f, 0.20000f, 0x80}
,{-14.00000f, 0.20000f, -15.00000f, -0.50000f, 0x80}
,{-15.00000f, -0.50000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, -7.00000f, -2.00000f, 0x80}
,{-7.00000f, -2.00000f, -6.00000f, -1.50000f, 0x80}
,{-6.00000f, -1.50000f, -6.00000f, -0.50000f, 0x80}
};

shapeSegment_t trans3SegsTransparent[] = {
{0.00000f, -0.50000f, -6.00000f, -0.50000f, 0x60}
};

shapeSegment_t trans3SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 0.40000f, 0xff}
};

shapeSegment_t trans3SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0xff}
,{-12.00000f, 0.40000f, -15.00000f, 0.40000f, 0xff}
};

shapeSegment_t trans4Segs[] = {
{-6.00000f, -0.50000f, -7.00000f, 0.00000f, 0x80}
,{-7.00000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 0.20000f, 0x80}
,{-13.00000f, 0.20000f, -14.00000f, 0.20000f, 0x80}
,{-14.00000f, 0.20000f, -15.00000f, -0.50000f, 0x80}
,{-15.00000f, -0.50000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, -7.00000f, -2.00000f, 0x80}
,{-7.00000f, -2.00000f, -6.00000f, -1.50000f, 0x80}
,{-6.00000f, -1.50000f, -6.00000f, -0.50000f, 0x80}
,{0.00000f, 0.00000f, 0.00000f, 0.00000f, 0x80}
,{0.00000f, 0.00000f, 0.00000f, -0.50000f, 0x80}
,{0.00000f, -0.50000f, 0.00000f, -1.50000f, 0x80}
,{0.00000f, -1.50000f, 0.00000f, -2.00000f, 0x80}
,{0.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
};

shapeSegment_t trans4SegsTransparent[] = {
{0.00000f, -0.50000f, -6.00000f, -0.50000f, 0x60}
};

shapeSegment_t trans4SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 0.40000f, 0xff}
};

shapeSegment_t trans4SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0xff}
,{-12.00000f, 0.40000f, -15.00000f, 0.40000f, 0xff}
};

shapeSegment_t trans5Segs[] = {
{-8.00000f, -0.50000f, -9.00000f, 0.00000f, 0x80}
,{-9.00000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 0.20000f, 0x80}
,{-13.00000f, 0.20000f, -14.00000f, 0.20000f, 0x80}
,{-14.00000f, 0.20000f, -15.00000f, -0.50000f, 0x80}
,{-15.00000f, -0.50000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, -9.00000f, -2.00000f, 0x80}
,{-9.00000f, -2.00000f, -8.00000f, -1.50000f, 0x80}
,{-8.00000f, -1.50000f, -8.00000f, -0.50000f, 0x80}
,{0.00000f, 0.00000f, -1.00000f, 0.00000f, 0x80}
,{-1.00000f, 0.00000f, -2.00000f, -0.50000f, 0x80}
,{-2.00000f, -0.50000f, -2.00000f, -1.50000f, 0x80}
,{-2.00000f, -1.50000f, -1.00000f, -2.00000f, 0x80}
,{-1.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
};

shapeSegment_t trans5SegsTransparent[] = {
{-2.00000f, -0.50000f, -8.00000f, -0.50000f, 0x60}
};

shapeSegment_t trans5SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 0.40000f, 0xff}
};

shapeSegment_t trans5SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0xff}
,{-12.00000f, 0.40000f, -15.00000f, 0.40000f, 0xff}
};

shapeSegment_t trans6Segs[] = {
{-6.00000f, -0.50000f, -7.00000f, 0.00000f, 0x80}
,{-7.00000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 3.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, -0.50000f, 0x80}
,{-15.00000f, -0.50000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, -7.00000f, -2.00000f, 0x80}
,{-7.00000f, -2.00000f, -6.00000f, -1.50000f, 0x80}
,{-6.00000f, -1.50000f, -6.00000f, -0.50000f, 0x80}
};

shapeSegment_t trans6SegsTransparent[] = {
{0.00000f, -0.50000f, -6.00000f, -0.50000f, 0x60}
};

shapeSegment_t trans6SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0x60}
};

shapeSegment_t trans6SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0x60}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0x60}
};

shapeSegment_t trans7Segs[] = {
{-6.00000f, -0.50000f, -7.00000f, 0.00000f, 0x80}
,{-7.00000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 3.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, -0.50000f, 0x80}
,{-15.00000f, -0.50000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, -7.00000f, -2.00000f, 0x80}
,{-7.00000f, -2.00000f, -6.00000f, -1.50000f, 0x80}
,{-6.00000f, -1.50000f, -6.00000f, -0.50000f, 0x80}
,{0.00000f, 0.00000f, 0.00000f, 0.00000f, 0x80}
,{0.00000f, 0.00000f, 0.00000f, -0.50000f, 0x80}
,{0.00000f, -0.50000f, 0.00000f, -1.50000f, 0x80}
,{0.00000f, -1.50000f, 0.00000f, -2.00000f, 0x80}
,{0.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
};

shapeSegment_t trans7SegsTransparent[] = {
{0.00000f, -0.50000f, -6.00000f, -0.50000f, 0x60}
};

shapeSegment_t trans7SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0x60}
};

shapeSegment_t trans7SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0x60}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0x60}
};

shapeSegment_t trans8Segs[] = {
{-8.00000f, -0.50000f, -9.00000f, 0.00000f, 0x80}
,{-9.00000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 3.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, -0.50000f, 0x80}
,{-15.00000f, -0.50000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, -9.00000f, -2.00000f, 0x80}
,{-9.00000f, -2.00000f, -8.00000f, -1.50000f, 0x80}
,{-8.00000f, -1.50000f, -8.00000f, -0.50000f, 0x80}
,{0.00000f, 0.00000f, -1.00000f, 0.00000f, 0x80}
,{-1.00000f, 0.00000f, -2.00000f, -0.50000f, 0x80}
,{-2.00000f, -0.50000f, -2.00000f, -1.50000f, 0x80}
,{-2.00000f, -1.50000f, -1.00000f, -2.00000f, 0x80}
,{-1.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
};

shapeSegment_t trans8SegsTransparent[] = {
{-2.00000f, -0.50000f, -8.00000f, -0.50000f, 0x60}
};

shapeSegment_t trans8SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0x60}
};

shapeSegment_t trans8SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0x60}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0x60}
};

shapeSegment_t tremplin1Segs[] = {
{0.00000f, 0.00000f, -1.00000f, 0.00000f, 0x80}
,{-1.00000f, 0.00000f, -2.00000f, 0.00100f, 0x80}
,{-2.00000f, 0.00100f, -3.00000f, 0.00100f, 0x80}
,{-3.00000f, 0.00100f, -4.00000f, 0.00000f, 0x80}
,{-4.00000f, 0.00000f, -5.00000f, 0.00000f, 0x80}
,{-5.00000f, 0.00000f, -6.00000f, 0.00100f, 0x80}
,{-6.00000f, 0.00100f, -7.00000f, 0.00100f, 0x80}
,{-7.00000f, 0.00100f, -8.00000f, 0.00000f, 0x80}
,{-8.00000f, 0.00000f, -9.00000f, 0.00000f, 0x80}
,{-9.00000f, 0.00000f, -10.00000f, 0.00100f, 0x80}
,{-10.00000f, 0.00100f, -11.00000f, 0.00100f, 0x80}
,{-11.00000f, 0.00100f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 3.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, 0.00000f, 0x80}
,{-15.00000f, 0.00000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
};

shapeSegment_t tremplin1SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0x80}
};

shapeSegment_t tremplin1SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0x80}
};

shapeSegment_t tremplin2Segs[] = {
{0.00000f, 0.00000f, -1.00000f, 0.00000f, 0x80}
,{-1.00000f, 0.00000f, -2.00000f, -1.00000f, 0x80}
,{-2.00000f, -1.00000f, -3.00000f, -1.00000f, 0x80}
,{-3.00000f, -1.00000f, -4.00000f, 0.00000f, 0x80}
,{-4.00000f, 0.00000f, -5.00000f, 0.00000f, 0x80}
,{-5.00000f, 0.00000f, -6.00000f, -1.00000f, 0x80}
,{-6.00000f, -1.00000f, -7.00000f, -1.00000f, 0x80}
,{-7.00000f, -1.00000f, -8.00000f, 0.00000f, 0x80}
,{-8.00000f, 0.00000f, -9.00000f, 0.00000f, 0x80}
,{-9.00000f, 0.00000f, -10.00000f, -1.00000f, 0x80}
,{-10.00000f, -1.00000f, -11.00000f, -1.00000f, 0x80}
,{-11.00000f, -1.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 0.40000f, 0x80}
,{-13.00000f, 0.40000f, -14.00000f, 0.40000f, 0x80}
,{-14.00000f, 0.40000f, -15.00000f, -0.60000f, 0x80}
,{-15.00000f, -0.60000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
};

shapeSegment_t tremplin2SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 0.40000f, 0xff}
};

shapeSegment_t tremplin2SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0xff}
,{-12.00000f, 0.40000f, -15.00000f, 0.40000f, 0xff}
};

shapeSegment_t tremplin3Segs[] = {
{0.00000f, 0.00000f, -1.00000f, 0.00000f, 0x80}
,{-1.00000f, 0.00000f, -2.00000f, 0.00000f, 0x80}
,{-2.00000f, 0.00000f, -3.00000f, 0.00000f, 0x80}
,{-3.00000f, 0.00000f, -4.00000f, 0.00000f, 0x80}
,{-4.00000f, 0.00000f, -5.00000f, 0.00000f, 0x80}
,{-5.00000f, 0.00000f, -6.00000f, 0.00000f, 0x80}
,{-6.00000f, 0.00000f, -7.00000f, 0.00000f, 0x80}
,{-7.00000f, 0.00000f, -8.00000f, 0.00000f, 0x80}
,{-8.00000f, 0.00000f, -9.00000f, 0.00000f, 0x80}
,{-9.00000f, 0.00000f, -10.00000f, 0.00000f, 0x80}
,{-10.00000f, 0.00000f, -11.00000f, 0.00000f, 0x80}
,{-11.00000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 3.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, 0.00000f, 0x80}
,{-15.00000f, 0.00000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
};

shapeSegment_t tremplin3SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0x80}
};

shapeSegment_t tremplin3SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0x80}
};

shapeSegment_t tunnel1Segs[] = {
{-1.00000f, -0.50000f, -1.50000f, 0.00000f, 0x80}
,{-1.50000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 2.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, 0.00000f, 0x80}
,{-15.00000f, 0.00000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
,{0.00000f, -0.50000f, -1.00000f, -0.50000f, 0xff}
,{-13.00000f, 2.00000f, -13.00000f, 3.00000f, 0xff}
};

shapeSegment_t tunnel1SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0xff}
};

shapeSegment_t tunnel1SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0xff}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0xff}
};

shapeSegment_t tunnel2Segs[] = {
{-1.00000f, 0.00000f, -1.50000f, 0.00000f, 0x80}
,{-1.50000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -12.67000f, 2.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, 0.00000f, 0x80}
,{-15.00000f, 0.00000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
,{0.00000f, 0.00000f, -1.00000f, 0.00000f, 0x81}
,{-12.67000f, 2.00000f, -13.00000f, 3.00000f, 0x81}
};

shapeSegment_t tunnel2SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0xff}
};

shapeSegment_t tunnel2SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0xff}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0xff}
};

shapeSegment_t tunnel3Segs[] = {
{-1.00000f, -0.50000f, -1.50000f, 0.00000f, 0x80}
,{-1.50000f, 0.00000f, -12.00000f, 0.00000f, 0x80}
,{-12.00000f, 0.00000f, -13.00000f, 2.00000f, 0x80}
,{-13.00000f, 3.00000f, -14.00000f, 3.00000f, 0x80}
,{-14.00000f, 3.00000f, -15.00000f, 0.00000f, 0x80}
,{-15.00000f, 0.00000f, -15.00000f, -1.00000f, 0x80}
,{-15.00000f, -1.00000f, -13.00000f, -2.00000f, 0x80}
,{-13.00000f, -2.00000f, 0.00000f, -2.00000f, 0x80}
,{0.00000f, -0.50000f, -1.00000f, -0.50000f, 0x81}
,{-13.00000f, 2.00000f, -13.00000f, 3.00000f, 0x81}
};

shapeSegment_t tunnel3SegsWall[] = {
{-12.00000f, 0.00000f, -12.00000f, 3.00000f, 0xff}
};

shapeSegment_t tunnel3SegsGround[] = {
{0.00010f, 0.00000f, -12.00000f, 0.00000f, 0xff}
,{-12.00000f, 3.00000f, -15.00000f, 3.00000f, 0xff}
};




shape_t Shapes[]={
{12.00f, {Startup1Segs,Startup1SegsTransparent,Startup1SegsWall,Startup1SegsGround}, {11,1,1,2} },
{12.00f, {Startup2Segs,Startup2SegsTransparent,Startup2SegsWall,Startup2SegsGround}, {11,1,1,2} },
{12.00f, {Startup3Segs,NULL,Startup3SegsWall,Startup3SegsGround}, {13,0,1,2} },
{12.00f, {trans1Segs,trans1SegsTransparent,trans1SegsWall,trans1SegsGround}, {10,1,1,2} },
{12.00f, {trans2Segs,trans2SegsTransparent,trans2SegsWall,trans2SegsGround}, {10,1,1,2} },
{12.00f, {trans3Segs,trans3SegsTransparent,trans3SegsWall,trans3SegsGround}, {10,1,1,2} },
{12.00f, {trans4Segs,trans4SegsTransparent,trans4SegsWall,trans4SegsGround}, {15,1,1,2} },
{12.00f, {trans5Segs,trans5SegsTransparent,trans5SegsWall,trans5SegsGround}, {15,1,1,2} },
{12.00f, {trans6Segs,trans6SegsTransparent,trans6SegsWall,trans6SegsGround}, {10,1,1,2} },
{12.00f, {trans7Segs,trans7SegsTransparent,trans7SegsWall,trans7SegsGround}, {15,1,1,2} },
{12.00f, {trans8Segs,trans8SegsTransparent,trans8SegsWall,trans8SegsGround}, {15,1,1,2} },
{12.00f, {tremplin1Segs,NULL,tremplin1SegsWall,tremplin1SegsGround}, {18,0,1,2} },
{12.00f, {tremplin2Segs,NULL,tremplin2SegsWall,tremplin2SegsGround}, {18,0,1,2} },
{12.00f, {tremplin3Segs,NULL,tremplin3SegsWall,tremplin3SegsGround}, {18,0,1,2} },
{12.00f, {tunnel1Segs,NULL,tunnel1SegsWall,tunnel1SegsGround}, {10,0,1,2} },
{12.00f, {tunnel2Segs,NULL,tunnel2SegsWall,tunnel2SegsGround}, {10,0,1,2} },
{12.00f, {tunnel3Segs,NULL,tunnel3SegsWall,tunnel3SegsGround}, {10,0,1,2} }
};

