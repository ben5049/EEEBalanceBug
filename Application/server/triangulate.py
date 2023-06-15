from math import cos, sin, pi
# # returns rover position from (0,0) 
# ang1 is bearing to light 1 (placed at origin)
# ang2 and ang3 are bearings to light 2 and light 3
# L1, L2 are 2.4m away. L1, L3 are 3.6m away
# L1 - - - - - - - - - - L2
# -
# -
# -              R
# - 
# - 
# - 
# - 
# - 
# - 
# L3
def triangulate(ang1, ang2, ang3):
    theta1 = 360-ang1
    x1 = 2.4*sin(theta1*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)
    y1 = 2.4*cos(theta1*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)

    theta1 = ang1-270
    ang3 = 270-ang3
    x2 = 3.6*sin((90-ang3)*pi/180)*cos(theta1*pi/180)/sin((theta1+ang3)*pi/180)
    y2 = 3.6*sin((90-ang3)*pi/180)*sin(theta1*pi/180)/sin((theta1+ang3)*pi/180)
    return (x1+x2)*500, (y1+y2)*500 # convert to mm

