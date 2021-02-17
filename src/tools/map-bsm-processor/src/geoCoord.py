"""
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  cv-work-zone-controller.py
  Created by: Larry Head
  University of Arizona   
  College of Engineering
  
This class converts WGS-84 coordinates to and from planar coordinates given a reference location. 
Only one instance is needed for each local planar region of interest. 
Call it with the reference locaiton to create an object, then use the object to 
convert other geo locations to planar coordinates. 

Reference Jay Farrell and Matt Barth, "The Global Positioning System & Inertial Navigation", McGraw-Hill, 
ISBN: 0-07-022045-X, 1998

"""

import math
# from src.common.Position3D import Position3D

from Position3D import Position3D


# algorithm parameters
a = 6378137.0 
b = 6356752.3142
f= 1.0 - (b/a)
inv_f = 1.0/f 
e = math.sqrt(f*(2-f)) 
pi = math.pi

class GeoCoord:
    def __init__(self, ref_position:Position3D):
        self.latitude_ref = ref_position.latitude_DecimalDegree
        self.longitude_ref = ref_position.longitude_DecimalDegree
        self.elevation_ref = ref_position.elevation_Meter
        self.R_sinLat = math.sin(self.latitude_ref*pi/180.0)
        self.R_cosLat = math.cos(self.latitude_ref*pi/180.0)
        self.R_sinLong = math.sin(self.longitude_ref*pi/180.0)
        self.R_cosLong = math.cos(self.longitude_ref*pi/180.0)
        self.ex_ref, self.ey_ref, self.ez_ref = self.lla2ecef(ref_position)


    def lla2ecef(self, pos:Position3D):
        # convert to radians
        latitude_r = pi*pos.latitude_DecimalDegree/180.0 
        longitude_r = pi*pos.longitude_DecimalDegree/180.0 
        
	    # compute constants
        N = a/math.sqrt(1.0 -math.pow(e*math.sin(latitude_r), 2)) 
	    # compute ECEF coordiates


        ex = (N+pos.elevation_Meter)*math.cos(latitude_r)*math.cos(longitude_r) 
        ey = (N+pos.elevation_Meter)*math.cos(latitude_r)*math.sin(longitude_r) 
        ez = (N*(1-math.pow(e, 2))+pos.elevation_Meter)*math.sin(latitude_r) 
        return ex, ey, ez

    def lla2local(self, pos:Position3D):
        ex, ey, ez = self.lla2ecef(pos)
        x, y, z = self.ecef2local(ex, ey, ez)
        return x, y, z

    def ecef2local(self, ex, ey, ez):
        R_sinLat = self.R_sinLat
        R_cosLat= self.R_cosLat
        R_sinLong = self.R_sinLong
        R_cosLong = self.R_cosLong
        ex_ref = self.ex_ref
        ey_ref = self.ey_ref
        ez_ref = self.ez_ref
        
        x=(-R_sinLat*R_cosLong)*(ex-ex_ref)+(-R_sinLat*R_sinLong)*(ey-ey_ref)+R_cosLat*(ez-ez_ref)
        y=(-R_sinLong)*(ex-ex_ref)+(R_cosLong)*(ey-ey_ref)+0
        z=(-R_cosLat*R_cosLong)*(ex-ex_ref)+(-R_cosLat*R_sinLong)*(ey-ey_ref)+(-R_sinLat)*(ez-ez_ref)
        return x, y, z

if __name__ == '__main__':
    pos = Position3D(32.000, -112.000, 100)
    geo = GeoCoord(pos)
    # simple self test code;
    print("a = ", a)
    print("b = ", b)
    print("f = ", f)
    print("inv_f = ", inv_f)
    print("e = ", e)
    print("pi = ", pi)

    print("calling lla2ecef")
    ex, ey, ez = geo.lla2ecef(pos)
    print (f'x = {ex:.8f}, y = {ey: .8f}, z = {ez: .8f}')

    print("calling ecef2local")
    x, y, z = geo.ecef2local(ex, ey, ez)
    # This should return (0, 0, 0) - or (0, 0, altitude - but on plane z doesn't matter. 
    print(f'x = {x: .8f}, y = {y: .8f}, z = {z: .8f}')

    #Speedway Test
    speedway_lat_ref =  32.235891
    speedway_long_ref = -110.952227
    speedway_elev_ref = 729.0
    speedway_pos = Position3D(speedway_lat_ref, speedway_long_ref, speedway_elev_ref)
    speedway_geo = GeoCoord(speedway_pos)
    ex, ey, ez = speedway_geo.lla2ecef(speedway_pos)
    print (f'x = {ex:.8f}, y = {ey: .8f}, z = {ez: .8f}')

    print("calling ecef2local")
    x, y, z = speedway_geo.ecef2local(ex, ey, ez)
    # This should return (0, 0, 0) - or (0, 0, altitude - but on plane z doesn't matter. 
    print(f'x = {x: .8f}, y = {y: .8f}, z = {z: .8f}')

    #now, pick a point about 100 meters east on Speedway
    speedway_lat =  32.235884
    speedway_long = -110.951133
    speedway_elev = 729.0
    speedway_100E = Position3D(speedway_lat, speedway_long, speedway_elev)
    sx, sy, sz = speedway_geo.lla2local(speedway_100E)
    print(f'sx = {sx: .8f}, sy = {sy: .8f}, sz = {sz: .8f}')
    distance = 103.71 
    newdistance = math.sqrt(sx**2+sy**2)
    print(f'google earth says = {distance: .5f} and my calculation says = {newdistance: .5f}')
    

    #check against Example, p. 27 = Farrell and Barth

    la_pos = Position3D(34.0, -117.333569, 251.702)
    la_geo = GeoCoord(la_pos)
    lax, lay, laz = la_geo.lla2ecef(la_pos)
    print(lax, lay, laz)
    print(f'lax = {lax: .8f}, lay = {lay: .8f}, laz = {laz: .8f}')







