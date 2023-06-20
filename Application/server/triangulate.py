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
DELTA = 0.05
LENGTH = 1 # 2.4
WIDTH = 1  # 3.6
def triangulate(ang1, ang2, ang3):
    theta1 = 360-ang1
    x1 = LENGTH*sin(theta1*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)
    y1 = LENGTH*cos(theta1*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)

    theta1 = ang1-270
    theta3 = 270-ang3
    x2 = WIDTH*sin((90-theta3)*pi/180)*cos(theta1*pi/180)/sin((theta1+theta3)*pi/180)
    y2 = WIDTH*sin((90-theta3)*pi/180)*sin(theta1*pi/180)/sin((theta1+theta3)*pi/180)
    
    xdiff = abs(x1-x2)
    ydiff = abs(y1-y2)
    print(xdiff, ydiff)
    # shift angle anticlockwise and calculate new x1, y1, x2, y2
    ang1-=DELTA
    ang2-=DELTA
    ang3-=DELTA
    theta1 = 360-ang1
    x1_left = LENGTH*sin(theta1*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)
    y1_left = LENGTH*cos(theta1*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)

    theta1 = ang1-270
    theta3 = 270-ang3
    x2_left = WIDTH*sin((90-theta3)*pi/180)*cos(theta1*pi/180)/sin((theta1+theta3)*pi/180)
    y2_left = WIDTH*sin((90-theta3)*pi/180)*sin(theta1*pi/180)/sin((theta1+theta3)*pi/180)

    # shift angles clockwise and calculate new x1, y1, x2, y2
    ang1+=2*DELTA
    ang2+=2*DELTA
    ang3+=2*DELTA

    theta1 = 360-ang1
    x1_right = LENGTH*sin(theta1*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)
    y1_right = LENGTH*cos(theta1*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)

    theta1 = ang1-270
    theta3 = 270-ang3
    x2_right= WIDTH*sin((90-theta3)*pi/180)*cos(theta1*pi/180)/sin((theta1+theta3)*pi/180)
    y2_right = WIDTH*sin((90-theta3)*pi/180)*sin(theta1*pi/180)/sin((theta1+theta3)*pi/180)

    ang1-=DELTA
    ang2-=DELTA
    ang3-=DELTA

    while xdiff > abs(x1_left-y1_left) or ydiff > abs(y1_left-y2_left) or xdiff > abs(x1_right-y1_right) or ydiff > abs(y1_right-y2_right):
        xdiff = abs(x1-x2)
        ydiff = abs(y1-y2)
        # shift angle anticlockwise and calculate new x1, y1, x2, y2
        ang1-=DELTA
        ang2-=DELTA
        ang3-=DELTA
        theta1 = 360-ang1
        x1_left = LENGTH*sin(theta1*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)
        y1_left = LENGTH*cos(theta1*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)

        theta1 = ang1-270
        theta3 = 270-ang3
        x2_left = WIDTH*sin((90-theta3)*pi/180)*cos(theta1*pi/180)/sin((theta1+theta3)*pi/180)
        y2_left = WIDTH*sin((90-theta3)*pi/180)*sin(theta1*pi/180)/sin((theta1+theta3)*pi/180)

        # shift angles clockwise and calculate new x1, y1, x2, y2
        ang1+=2*DELTA
        ang2+=2*DELTA
        ang3+=2*DELTA

        theta1 = 360-ang1
        x1_right = LENGTH*sin(theta1*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)
        y1_right = LENGTH*cos(theta1*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)

        theta1 = ang1-270
        theta3 = 270-ang3
        x2_right= WIDTH*sin((90-theta3)*pi/180)*cos(theta1*pi/180)/sin((theta1+theta3)*pi/180)
        y2_right = WIDTH*sin((90-theta3)*pi/180)*sin(theta1*pi/180)/sin((theta1+theta3)*pi/180)


        # if anticlockwise values are better, set best values to anticlockwise values, 
        # change angles to anticlockwise angles and iterate
        if abs(x1_left-x2_left)<xdiff and abs(y1_left-y2_left)<ydiff:
            x1 = x1_left
            y1 = y1_left
            x2 = x2_left
            y2 = y2_left
            ang1-=2*DELTA
            ang2-=2*DELTA
            ang3-=2*DELTA
        # if clockwise values are better, set best values to clockwise values 
        # (angles already clockwise) and iterate
        elif abs(x1_right-x2_right)<xdiff and abs(y1_right-y2_right)<ydiff:
            x1 = x1_right
            y1 = y1_right
            x2 = x2_right
            y2 = y2_right
        # if changing angles either way is worse, return best values
        else:
            break
    return (x1+x2)*500, (y1+y2)*500 # convert to mm

# print(triangulate(-45, 45, 225))
