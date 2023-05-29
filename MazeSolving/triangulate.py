from math import cos, sin, pi
import cv2
import numpy as np
# checks if beacon is in centre of image
# if below a threshold, return None
# if not below threshold, check if near centre of image
# if it is, return ang. else return none
def findBeacon(img, ang):
    img = cv2.imread("red_right.png")
    red_pixels = np.argwhere(cv2.inRange(img, (1, 1, 200), (100, 100, 250)))
    mask = cv2.inRange(img, (1, 1, 200), (100, 100, 250))
    redx = []
    redy = []

    for px, py in red_pixels:
        redx.append(px)
        redy.append(py)
    redx = sorted(redx)
    redy = sorted(redy)
    av = [0,0]
    for i in range(1,len(redx)-1):
        if redx[i-1] == redx[i]-1 and redx[i+1]==redx[i]+1:
            av[0]+=1.5*redx[i]
        elif redx[i-1] == redx[i]-1 or redx[i+1]==redx[i]+1:
            av[0]+=1.25*redx[i]
        else:
            av[0]+=redx[i]
        
        if redy[i-1] == redy[i]-1 and redy[i+1]==redy[i]+1:
            av[1]+=1.5*redy[i]
        elif redy[i-1] == redy[i]-1 or redy[i+1]==redy[i]+1:
            av[1]+=1.25*redy[i]
        else:
            av[1]+=redy[i]
    av[0] = int(av[0]/len(redx))
    av[1] = int(av[1]/len(redx))

    print(av)
    test = cv2.circle(img, (747,256), 20, (255,0,0), 2)
    cv2.imwrite("test.png", test)
    print(img[av[0]][av[1]])




        
    return 
     

# returns rover position from (0,0) 
# ang1 is bearing to light 1 (placed at origin)
# ang2 and ang3 are bearings to light 2 and light 3
# light 2 and light 3 are exactly 1m away from light 1 and perpendicular
# L1 - - - - - - - - - - L2
# -
# -
# -
# - 
# - 
# - 
# - 
# - 
# - 
# L3
def triangulate(ang1, ang2, ang3):
    theta1 = 360-ang1
    x1 = cos((90-theta1)*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)
    y1 = sin((90-theta1)*pi/180)*sin((90-ang2)*pi/180)/sin((theta1+ang2)*pi/180)

    theta1 = ang1-270
    ang3 = 270-ang3
    x2 = sin((90-ang3)*pi/180)*cos(theta1*pi/180)/sin((theta1+ang2)*pi/180)
    y2 = sin((90-ang3)*pi/180)*sin(theta1*pi/180)/sin((theta1+ang2)*pi/180)
    return (x1+x2)/2, (y1+y2)/2

print(triangulate(302, 48, 223))
findBeacon(0, 0)