#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shapefil.h"

/*
  Code to display Census shapefiles.
  Copyright (C) <2009>  <Joshua Justice>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

  The Shapelib library is licensed under the GNU Lesser General Public License.
  A copy of the GNU LGPL can be found on http://www.gnu.org/licenses/lgpl-3.0.txt .
  For information on Shapelib, see http://shapelib.maptools.org/ .

  EULA: The Graphics Gems code is copyright-protected. 
  In other words, you cannot claim the text of the code as your own and resell it. 
  Using the code is permitted in any program, product, or library, non-commercial or commercial. 
  Giving credit is not required, though is a nice gesture. 
  The code comes as-is, and if there are any flaws or problems with any Gems code, nobody involved
  with Gems - authors, editors, publishers, or webmasters - are to be held responsible. 
  Basically, don't be a jerk, and remember that anything free comes with no guarantee. 
*/



/*  
  polyCentroid: Calculates the centroid (xCentroid, yCentroid) and area
  of a polygon, given its vertices (x[0], y[0]) ... (x[n-1], y[n-1]). It
  is assumed that the contour is closed, i.e., that the vertex following
  (x[n-1], y[n-1]) is (x[0], y[0]).  The algebraic sign of the area is
  positive for counterclockwise ordering of vertices in x-y plane;
  otherwise negative.

  Returned values:  0 for normal execution;  1 if the polygon is
  degenerate (number of vertices < 3);  and 2 if area = 0 (and the
  centroid is undefined).
*/

int polyCentroid(double x[], double y[], int n,
		 double *xCentroid, double *yCentroid, double *area){
     register int i, j;
     double ai, atmp = 0, xtmp = 0, ytmp = 0;
     if (n < 3) return 1;
     for (i = n-1, j = 0; j < n; i = j, j++){
	  ai = x[i] * y[j] - x[j] * y[i];
	  atmp += ai;
	  xtmp += (x[j] + x[i]) * ai;
	  ytmp += (y[j] + y[i]) * ai;
     }
     *area = atmp / 2;
     if (atmp != 0){
	  *xCentroid =	xtmp / (3 * atmp);
	  *yCentroid =	ytmp / (3 * atmp);
	  return 0;
     }
     return 2;
} //end Graphics Gems code


void svg_header(FILE *svg){
  fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n", svg);
  fputs("<svg\n\txmlns:svg=\"http://www.w3.org/2000/svg\"\n", svg);
  fputs("\txmlns=\"http://www.w3.org/2000/svg\"\n", svg);
  fputs("\tversion=\"1.0\"\n", svg);
  fputs("\twidth=\"360000\"\n", svg);
  fputs("\theight=\"180000\"\n", svg);
  fputs("\tid=\"svg2\">\n", svg);
  fputs("\t<defs\n\t\tid=\"defs1\" />\n", svg);
  fputs("\t<g\n\t\tid=\"layer1\">\n", svg);
}

void svg_polygon(SHPObject block, FILE *svg){
  int i,j,jLim;
  double x,y;
  fputs("\t\t<path\n\t\t\td=\"", svg);  
  for(i=0;i<block.nParts;i++){
    if(i==block.nParts-1){
      jLim=block.nVertices-1;
    }else{
      jLim=block.panPartStart[i+1]-2;
    }
    for(j=block.panPartStart[i];j<jLim;j++){
      //draw coordinates at padfX[j] etc.
      if(j==block.panPartStart[i]){
        fputs("M ",svg); //not having the \n is deliberate
      }else{
        fputs("L ",svg); //no \n is also deliberate here
      }
      x=(block.padfX[j]+180)*10000;
      y=(block.padfY[j]-90)*-10000; //SVG has y-down
      fprintf(svg, "%f %f ",x,y);
    }
  }
  fprintf(svg,"\"\n\t\t\tid=\"path%d\"\n",block.nShapeId);
  fputs("\t\t\tstyle=\"fill:#ffffff;fill-rule:evenodd;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"/>",svg);
}

void svg_footer(FILE *svg){
  fputs("\t</g>\n", svg);
  fputs("</svg>", svg);
}

int main(){
  int entityCount;
  int shapeType;
  double padfMinBound[4];
  double padfMaxBound[4];
  int i;
  //For now, we'll use this. Later on, this will change.
  //char sf_name[] = "/home/josh/Desktop/FultonCoData/Fultoncombinednd.shp";
  //for clamps!
  char sf_name[] = "/home/joshua/FultonCoData/Fultoncombinednd.shp";
  
  SHPHandle handle = SHPOpen(sf_name, "rb");


  int fn_len = strlen(sf_name);
  char svg_filename[fn_len];
  FILE *svg;
  strcpy(svg_filename, sf_name);
  svg_filename[fn_len-2] = 'v';
  svg_filename[fn_len-1] = 'g';

  SHPGetInfo(handle, &entityCount, &shapeType, padfMinBound, padfMaxBound);
  printf("There are %d entities, of type %d\n", entityCount, shapeType);
  printf("Filename is: %s \n", svg_filename);
  
  printf("Allocating %ld bytes of memory\n", entityCount*sizeof(SHPObject *));
  SHPObject **shapeList = malloc(entityCount*sizeof(SHPObject *));
  double xCentList[entityCount];
  double yCentList[entityCount];
  double areaList[entityCount];
  //populate the shapeList
  for(i=0; i<entityCount; i++){
    shapeList[i] = SHPReadObject(handle,i);
  }
  printf("Shapelist populated.\n");
  //delete file if it exists
  remove(svg_filename);
  //set up the SVG file pointer
  svg = fopen(svg_filename, "a+");
  printf("SVG file opened for writing.\n");

  //TODO: load pairs from GAL file
  






  //find centroids for every block
  for(i=0; i<entityCount; i++){
       int lastPoint;
       double *xCentroid;
       double *yCentroid;
       double *area;
       int status;
       SHPObject block = *shapeList[i];
       //Note that we're going to disregard holes, etc.
       if(block.nParts>1){
            lastPoint = block.panPartStart[1]-1;
       }else{
            lastPoint = block.nVertices;
       }
       printf("%d\n",lastPoint); //try to find the segfault
       status = polyCentroid(block.padfX, block.padfY, lastPoint, xCentroid, yCentroid, area);
       printf("%d\n",status); //Why do I think it'll be here?
       xCentList[i] = *xCentroid;
       yCentList[i] = *yCentroid;
       areaList[i] = *area;
  }
  //test code:
  for(i=0; i<10; i++){
       printf("X: %f Y: %f \n", xCentList[i], yCentList[i]);
  }



  //write header
  svg_header(svg);

  //write individual polygons
  for(i=0; i<entityCount; i++){
    svg_polygon(*shapeList[i], svg);
  }



  printf("SVG header printed.\n");
  


  //TODO: write pairs of centroids
  

  
  //write footer
  svg_footer(svg);
  printf("SVG footer printed.\n");
  for(i=0; i<entityCount; i++){
    SHPDestroyObject(shapeList[i]);
  }
  SHPClose(handle);
  fclose(svg);
  return 0;
}

