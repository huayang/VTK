/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPolyhedron.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPolyhedron - a 3D cell defined by a set of polygonal faces
// .SECTION Description
// vtkPolyhedron is a concrete implementation that represents a 3D cell
// defined by a set of polygonal faces. The polyhedron should be watertight,
// non-self-intersecting and manifold (each edge is used twice).
//
// Interpolation functions and weights are defined / computed using the
// method of Mean Value Coordinates (MVC). See the VTK class
// vtkMeanValueCoordinatesInterpolator for more information.
//
// The class assumes that the polyhedron is non-convex. However, the
// polygonal faces should be planar. Non-planar polygonal faces will
// definitely cause problems, especially in severely warped situations.

// .SECTION See Also
// vtkCell3D vtkConvecPointSet vtkMeanValueCoordinatesInterpolator

#ifndef __vtkPolyhedron_h
#define __vtkPolyhedron_h

#include "vtkCell3D.h"
#include "vtkType.h"
#include <vtkstd/map>
#include <vtkstd/vector>

class vtkIdTypeArray;
class vtkCellArray;
class vtkTriangle;
class vtkQuad;
class vtkTetra;
class vtkPolygon;
class vtkLine;
//BTX
struct vtkPointIdMap;
//ETX
class vtkEdgeTable;
class vtkPolyData;
class vtkCellLocator;
class vtkGenericCell;
class vtkPointLocator;


class VTK_FILTERING_EXPORT vtkPolyhedron : public vtkCell3D
{
public:
  // Description:
  // Standard new methods.
  static vtkPolyhedron *New();
  vtkTypeRevisionMacro(vtkPolyhedron,vtkCell3D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Special types.
  //BTX
  typedef vtkstd::vector<vtkIdType>                 IdVectorType;
  typedef vtkstd::map<vtkIdType, IdVectorType>      IdToIdVectorMapType;
  typedef vtkstd::pair<vtkIdType, IdVectorType>     IdToIdVectorPairType;
  typedef IdToIdVectorMapType::iterator             IdToIdVectorMapIteratorType;
  typedef vtkstd::map<vtkIdType, vtkIdType>         IdToIdMapType;
  typedef vtkstd::pair<vtkIdType, vtkIdType>        IdToIdPairType;
  //ETX

  // Description:
  // See vtkCell3D API for description of these methods.
  virtual void GetEdgePoints(int vtkNotUsed(edgeId), int* &vtkNotUsed(pts)) {}
  virtual void GetFacePoints(int vtkNotUsed(faceId), int* &vtkNotUsed(pts)) {}
  virtual double *GetParametricCoords();

  // Description:
  // See the vtkCell API for descriptions of these methods.
  virtual int GetCellType() {return VTK_POLYHEDRON;}

  // Description:
  // This cell requires that it be initialized prior to access.
  virtual int RequiresInitialization() {return 1;}
  virtual void Initialize();

  // Description:
  // A polyhedron is represented internally by a set of polygonal faces.
  // These faces can be processed to explicitly determine edges. 
  virtual int GetNumberOfEdges();
  virtual vtkCell *GetEdge(int);
  virtual int GetNumberOfFaces();
  virtual vtkCell *GetFace(int faceId);

  // Description:
  // Satisfy the vtkCell API. This method contours by triangulating the
  // cell and then contouring the resulting tetrahedra.
  virtual void Contour(double value, vtkDataArray *pointScalars,
                       vtkIncrementalPointLocator *locator, vtkCellArray *verts,
                       vtkCellArray *lines, vtkCellArray *polys,
                       vtkPointData *inPd, vtkPointData *outPd,
                       vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd);

  // Description:
  // Satisfy the vtkCell API. This method clips by triangulating the
  // cell and then adding clip-edge intersection points into the
  // triangulation; extracting the clipped region.
  virtual void Clip(double value, vtkDataArray *pointScalars,
                    vtkPointLocator *locator, vtkCellArray *connectivity,
                    vtkPointData *inPd, vtkPointData *outPd,
                    vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd,
                    int insideOut);

  // Description:
  // Satisfy the vtkCell API. The subId is ignored and zero is always
  // returned.  the parametric coordinates pcoords are normalized values in
  // the bounding box of the polyhedron. The weights are determined by
  // evaluating the MVC coordinates. The dist is always zero if the point x[3]
  // is inside the polyhedron; otherwise it's the distance to the surface.
  virtual int EvaluatePosition(double x[3], double* closestPoint,
                               int& subId, double pcoords[3],
                               double& dist2, double *weights);

  // Description:
  // The inverse of EvaluatePosition. Note the weights should be the MVC
  // weights.
  virtual void EvaluateLocation(int& subId, double pcoords[3], double x[3],
                                double *weights);

  // Description:
  // Intersect the line (p1,p2) with a given tolerance tol to determine a
  // point of intersection x[3] with parametric coordinate t along the
  // line. The parametric coordinates are returned as well (subId can be
  // ignored). Returns the number of intersection points.
  virtual int IntersectWithLine(double p1[3], double p2[3], double tol, double& t,
                                double x[3], double pcoords[3], int& subId);

  // Description:
  // Use vtkOrderedTriangulator to tetrahedralize the polyhedron mesh. This 
  // method works well for a convex polyhedron but may return wrong result
  // in a concave case.
  // Once triangulation has been performed, the results are saved in ptIds and
  // pts. The ptIds is a vtkIdList with 4xn number of ids (n is the 
  // number of result tetrahedrons). The first 4 represent the ids of the points
  // of the first tetrahedron, the second 4 ids represents the ids for the 
  // second tetrahedron and so on. The ids in ptIds range from 0 to m-1, where m
  // is the number of points in the original polyhedron. 
  // While the TetsPoints stores the coordinate of 4xn number of points. One
  // original point may be stored multiple times if it is shared by multiple
  // tetrahedrons. The points in TetsPoints are ordered the same as they are 
  // listed in Tets. 
  virtual int Triangulate(int index, vtkIdList *ptIds, vtkPoints *pts);

  // Description:
  // Computes derivatives at the point specified by the parameter coordinate.
  // Current implementation uses all vertices and subId is not used.
  // To accelerate the speed, the future implementation can triangulate and 
  // extract the local tetrahedron from subId and pcoords, then evaluate 
  // derivatives on the local tetrahedron. 
  virtual void Derivatives(int subId, double pcoords[3], double *values,
                           int dim, double *derivs);

  // Description:
  // Find the boundary face closest to the point defined by the pcoords[3]
  // and subId of the cell (subId can be ignored).
  virtual int CellBoundary(int subId, double pcoords[3], vtkIdList *pts);

  // Description:
  // Return the center of the cell in parametric coordinates. In this cell,
  // the center of the bounding box is returned.
  virtual int GetParametricCenter(double pcoords[3]);

  // Description:
  // A polyhedron is a full-fledged primary cell.
  int IsPrimaryCell() {return 1;}

  // Description:
  // Compute the interpolation functions/derivatives
  // (aka shape functions/derivatives). Here we use the MVC calculation
  // process to compute the interpolation functions.
  virtual void InterpolateFunctions(double x[3], double *sf);
  virtual void InterpolateDerivs(double x[3], double *derivs);

  // Description:
  // Methods supporting the definition of faces. Note that the GetFaces()
  // returns a list of faces in vtkCellArray form; use the method
  // GetNumberOfFaces() to determine the number of faces in the list.
  // The SetFaces() method is also in vtkCellArray form, except that it
  // begins with a leading count indicating the total number of faces in 
  // the list.
  virtual int RequiresExplicitFaceRepresentation() {return 1;}
  virtual void SetFaces(vtkIdType *faces);
  virtual vtkIdType *GetFaces();

  // Descriprion:
  // A method particular to vtkPolyhedron. It determines whether a point x[3]
  // is inside the polyhedron or not (returns 1 is the point is inside, 0
  // otherwise). The tolerance is expressed in normalized space; i.e., a
  // fraction of the size of the bounding box.
  int IsInside(double x[3], double tolerance);

  // Description:
  // Construct polydata if no one exist then return this->PolyData
  vtkPolyData* GetPolyData();
  
protected:
  vtkPolyhedron();
  ~vtkPolyhedron();

  // Internal classes for supporting operations on this cell
  vtkLine        *Line;
  vtkTriangle    *Triangle;
  vtkQuad        *Quad;
  vtkPolygon     *Polygon;
  vtkTetra       *Tetra;
  vtkIdTypeArray *GlobalFaces; //these are numbered in gloabl id space
  vtkIdTypeArray *FaceLocations;

  // vtkCell has the data members Points (x,y,z coordinates) and PointIds
  // (global cell ids corresponsing to cell canonical numbering (0,1,2,....)).
  // These data members are implicitly organized in canonical space, i.e., where
  // the cell point ids are (0,1,...,npts-1). The PointIdMap maps global point id 
  // back to these canonoical point ids.
  vtkPointIdMap  *PointIdMap;

  // If edges are needed. Note that the edge numbering is in
  // canonical space.
  int             EdgesGenerated; //true/false
  vtkEdgeTable   *EdgeTable; //keep track of all edges
  vtkIdTypeArray *Edges; //edge pairs kept in this list, in canonical id space
  int             GenerateEdges(); //method populates the edge table and edge array

  // If faces need renumbering into canonical numbering space these members
  // are used. When initiallly loaded, the face numbering uses global dataset
  // ids. Once renumbered, they are converted to canonical space.
  vtkIdTypeArray *Faces; //these are numbered in canonical id space
  int             FacesGenerated;
  void            GenerateFaces();

  // If triangulation has been performed. TriangulationPerformed flag will be
  // set to true. The result vtkIdList and vtkPoints will be stored. So the next
  // time the Triangulate() function is called, we don't need to run the whole 
  // triangulation process again.
  int             TriangulationPerformed;
  vtkIdList      *Tets;
  vtkPoints      *TetsPoints;

  // Bounds management
  int    BoundsComputed;
  void   ComputeBounds();
  void   ComputeParametricCoordinate(double x[3], double pc[3]);
  void   ComputePositionFromParametricCoordinate(double pc[3], double x[3]);

  // Members for supporting geometric operations
  int             PolyDataConstructed;
  vtkPolyData    *PolyData;
  vtkCellArray   *Polys;
  vtkIdTypeArray *PolyConnectivity;
  void            ConstructPolyData();
  int             LocatorConstructed;
  vtkCellLocator *CellLocator;
  void            ConstructLocator();
  vtkIdList      *CellIds;
  vtkGenericCell *Cell;


  void InternalContour(double value,
                       vtkIncrementalPointLocator *locator,
                       vtkDataArray *inScalars, 
                       vtkDataArray *outScalars,
                       vtkPointData *inPd, 
                       vtkPointData *outPd,
                       vtkCellArray *contourPolys,
                       IdToIdVectorMapType & faceToPointsMap,
                       IdToIdVectorMapType & pointToFacesMap,
                       IdToIdMapType & pointIdMap);

private:
  vtkPolyhedron(const vtkPolyhedron&);  // Not implemented.
  void operator=(const vtkPolyhedron&);  // Not implemented.
};

//----------------------------------------------------------------------------
inline int vtkPolyhedron::GetParametricCenter(double pcoords[3])
{
  pcoords[0] = pcoords[1] = pcoords[2] = 0.5;
  return 0;
}

#endif
