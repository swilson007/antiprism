/*
   Copyright (c) 2007, Roger Kaufman

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

      The above copyright notice and this permission notice shall be included
      in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

/*
   Name: elem_merge.cc
   Description: Sort vertices and faces indexes of an OFF file.
                Merge coincident vertices and/or faces.
   Project: Antiprism - http://www.antiprism.com
*/

#include <string.h>
#include <string>
#include <vector>
#include <algorithm>

#include "geom.h"
#include "transforms.h"
#include "math_utils.h"

using std::string;
using std::vector;


bool polygon_sort(vector<int> &polygon)
{
   bool reversed = false;
   vector<int> polygon_sorted;

   // Find lowest index make it the first polygon vertex index
   vector<int>::iterator iter = min_element(polygon.begin(), polygon.end()); 
   polygon_sorted.push_back(*iter);
   
   // Find rest of polygon indexes
   if ( iter < polygon.end() )
      polygon_sorted.insert(polygon_sorted.end(), (iter+1), polygon.end());
   if ( iter > polygon.begin() )
      polygon_sorted.insert(polygon_sorted.end(), polygon.begin(), iter);

   // reverse them if necessary
   iter = polygon_sorted.begin()+1;
   if ( *iter > polygon_sorted.back() ) {
      reverse(iter, polygon_sorted.end());
      reversed = true;
   }

   polygon.clear();
   polygon.assign(polygon_sorted.begin(), polygon_sorted.end());
   return reversed;
}

class vertexMap
{
public:
   int old_vertex;
   int new_vertex;
   vertexMap(int o, int n) : old_vertex(o), new_vertex(n) {}
};

bool cmp_vertexMap(const vertexMap &a, const vertexMap &b)
{
      return a.old_vertex < b.old_vertex;
}

void remap_faces(vector<vector<int> > &faces, const vector<vertexMap> &vm)
{
   for(unsigned int i=0;i<faces.size();i++)
      for(unsigned int j=0;j<faces[i].size();j++)
         faces[i][j] = vm[faces[i][j]].new_vertex;
}

class facesSort
{
public:
   vector<int> face;
   vector<int> face_with_cv;
   vector<int> equiv_faces;
   int face_no;
   bool deleted;
   bool reversed;
   facesSort(vector<int> f, int i, bool rev) :
      face(f), face_no(i), reversed(rev) { deleted = false; }
   facesSort(vector<int> f, vector<int> fcv, int i, bool rev) :
      face(f), face_with_cv(fcv), face_no(i), reversed(rev) { deleted = false; }
};

bool cmp_faces(const facesSort &a, const facesSort &b)
{
   if ( a.face.size() != b.face.size() )
      return a.face.size() < b.face.size();
   else
      return a.face < b.face;
}

void sort_faces(geom_if *geom, vector<vertexMap> *vm_no_cv,
      vector<vertexMap> *vm_with_cv, string delete_elems, char elem,
      map<int, set<int> > *equiv_elems = 0)
{
   vector<vector<int> > &faces =
         (elem=='f') ? *geom->get_faces() : *geom->get_edges();
   col_geom *cg = dynamic_cast<col_geom *>(geom);

   vector<facesSort> fs;
   vector<col_val> cv;

   // make a copy of unmapped faces with coincident vertices if necessary
   vector<vector<int> > faces_with_cv;
   if ( vm_with_cv->size() ) {
      faces_with_cv = faces;
      remap_faces(faces_with_cv, *vm_with_cv);
   }

   // use geom's face's and not a copy for no_cv
   if ( vm_no_cv->size() )
      remap_faces(faces, *vm_no_cv);

   // load face sort vector
   for(unsigned int i=0;i<faces.size();i++) {
      bool reversed;
      reversed = polygon_sort(faces[i]);
      if ( !faces_with_cv.size() )
         fs.push_back(facesSort(faces[i],i, reversed));
      else
      {
         polygon_sort(faces_with_cv[i]);
         fs.push_back(facesSort(faces[i],faces_with_cv[i],i, reversed));
      }
   }

   // don't need to find coinicident faces/edges if no map provided
   if ( vm_no_cv->size() ) {
      stable_sort(fs.begin(), fs.end(), cmp_faces);

      // erase coincident faces, if any, coincident edges always erased
      if ( elem=='e' || (elem=='f' && strchr(delete_elems.c_str(), 'f')) ) {
         if(equiv_elems && fs.size())
            fs[0].equiv_faces.push_back(fs[0].face_no);
         int cur_undeleted = 0;  // the first face in a set of equivalent faces
         for(unsigned int i=0;i<fs.size()-1;i++) {
            unsigned int j=i+1;
            if ( fs[i].face == fs[j].face )
               fs[j].deleted = true;
            else
               cur_undeleted = j;

            if(equiv_elems)
               fs[cur_undeleted].equiv_faces.push_back(fs[j].face_no);
         }
      }
   }

   // if coincient vertices are included we need to use/re-sort by those faces
   // but deleted flag will be preserved
   if ( !strchr(delete_elems.c_str(), 'v') ) {
      for(unsigned int i=0;i<fs.size();i++)
         fs[i].face = fs[i].face_with_cv;
      stable_sort(fs.begin(), fs.end(), cmp_faces);
   }

   // rewrite sorted face list, record colors for rebuilding color table
   bool restore_orient = (strchr(delete_elems.c_str(), 's')==0);
   faces.clear();
   for(unsigned int i=0;i<fs.size();i++) {
      if ( !fs[i].deleted ) {
         if(cg) {
            if(elem=='f')
               cv.push_back(cg->get_f_col(fs[i].face_no));
            else
               cv.push_back(cg->get_e_col(fs[i].face_no));
         }
         if(restore_orient && fs[i].reversed)
            reverse(fs[i].face.begin(), fs[i].face.end());
         faces.push_back(fs[i].face);
         if(equiv_elems) {
            (*equiv_elems)[faces.size()-1].insert( fs[i].equiv_faces.begin(),
                  fs[i].equiv_faces.end());
         }
      }
   }

   // rebuild color table
   if(cg) {
      if(elem=='f') {
         cg->clear_f_cols();
         for(unsigned int i=0;i<cv.size();i++)
            cg->set_f_col(i,cv[i]);
      }
      else {
         cg->clear_e_cols();
         for(unsigned int i=0;i<cv.size();i++)
            cg->set_e_col(i,cv[i]);
      }
   }
}

class vertSort
{
public:
   vec3d vert;
   int vert_no;
   bool deleted;
   vertSort(vec3d v, int i) : vert(v), vert_no(i) { deleted = false; }
};

bool cmp_verts(const vertSort &a, const vertSort &b, double epsilon)
{
   if ( !double_equality(a.vert[0],b.vert[0],epsilon) )
      return a.vert[0] < b.vert[0];
   else
   if ( !double_equality(a.vert[1],b.vert[1],epsilon) )
      return a.vert[1] < b.vert[1];
   else
      return a.vert[2] < b.vert[2];
}

class vert_cmp
{
public:
   double epsilon;
   vert_cmp(double ep): epsilon(ep) {}
   bool operator() (const vertSort &a, const vertSort &b) { return cmp_verts(a, b, epsilon); }
};

void sort_vertices(geom_if *geom, vector<vertexMap> *vm_no_cv,
      vector<vertexMap > *vm_with_cv, string delete_elems, double epsilon,
      map<int, set<int> > *equiv_elems = 0)
{
   vector<vec3d> &verts = *geom->get_verts();
   col_geom *cg = dynamic_cast<col_geom *>(geom);

   vector<vertSort> vs;
   vector<col_val> cv;

   for(unsigned int i=0;i<verts.size();i++)
      vs.push_back(vertSort(verts[i],i));
   stable_sort(vs.begin(), vs.end(), vert_cmp(epsilon));

   // build map from old to new vertices with coincident vertices included
   // if v is specified we will not be using it
   if ( !strchr(delete_elems.c_str(), 'v') ) {
      for(unsigned int i=0;i<vs.size();i++)
         vm_with_cv->push_back(vertexMap(vs[i].vert_no,i));
      stable_sort(vm_with_cv->begin(), vm_with_cv->end(), cmp_vertexMap);
   }

   // build map for coincident vertices if any deletion is to be done in vef.
   if ( strlen(delete_elems.c_str()) ) {
      int v = 0;
      for(unsigned int i=0;i<vs.size()-1;i++) {
         unsigned int j=i+1;
         if(compare(vs[i].vert, vs[j].vert, epsilon)==0) {
            // don't set delete flag if we will not be using it
            if ( strchr(delete_elems.c_str(), 'v') )
               vs[j].deleted = true;
            vm_no_cv->push_back(vertexMap(vs[i].vert_no,v));
         }
         else {
            vm_no_cv->push_back(vertexMap(vs[i].vert_no,v));
            v++;
         }
      }
      vm_no_cv->push_back(vertexMap(vs.back().vert_no,v));
      stable_sort(vm_no_cv->begin(), vm_no_cv->end(), cmp_vertexMap);
   }

   // rewrite sorted vertex list, record colors for rebuilding color table
   verts.clear();
   for(unsigned int i=0;i<vs.size();i++) {
      if ( !vs[i].deleted ) {
         if(cg)
            cv.push_back(cg->get_v_col(vs[i].vert_no));
         verts.push_back(vs[i].vert);
      }
      if(equiv_elems)
         (*equiv_elems)[verts.size()-1].insert(vs[i].vert_no);
   }

   // rebuild color table
   if(cg) {
      cg->clear_v_cols();
      for(unsigned int i=0;i<cv.size();i++)
         cg->set_v_col(i,cv[i]);
   }
}

bool sort_merge_elems(geom_if &geom, const char *merge_elems, double epsilon,
      vector<map<int, set<int> > > *equiv_elems, bool chk_congruence)
{
   // Congruence check expects a polyhedron with elements occurring
   // in coincident pairs, and exits early if this is not the case
   vector<map<int, set<int> > > equiv_elems_tmp;
   if(chk_congruence && !equiv_elems)
      equiv_elems = &equiv_elems_tmp;

   if(equiv_elems) {
      equiv_elems->clear();
      equiv_elems->resize(3);
   }

   unsigned int num_verts = geom.verts().size();
   unsigned int num_edges = geom.edges().size();
   unsigned int num_faces = geom.faces().size();

   vector<vertexMap> vm_no_cv, vm_with_cv;
   sort_vertices(&geom, &vm_no_cv, &vm_with_cv, merge_elems, epsilon,
         equiv_elems ? &(*equiv_elems)[0] : 0);
   if(chk_congruence && (*equiv_elems)[0].size()*2 != num_verts)
      return false;

   if(geom.get_edges()->size()>0)
      sort_faces(&geom, &vm_no_cv, &vm_with_cv, merge_elems, 'e',
         equiv_elems ? &(*equiv_elems)[1] : 0);
   if(chk_congruence && (*equiv_elems)[1].size()*2 != num_edges)
      return false;
   
   if(geom.get_faces()->size()>0)
      sort_faces(&geom, &vm_no_cv, &vm_with_cv, merge_elems, 'f',
         equiv_elems ? &(*equiv_elems)[2] : 0);
   if(chk_congruence && (*equiv_elems)[2].size()*2 != num_faces)
      return false;
   
   return true;
}

void sort_merge_elems(geom_if &geom, const char *merge_elems, double eps,
      vector<map<int, set<int> > > *equiv_elems)
{
   sort_merge_elems(geom, merge_elems, eps, equiv_elems, false);
}

bool check_congruence(const geom_if &geom1, const geom_if &geom2,
      double eps, vector<map<int, set<int> > > *equiv_elems)
{
   col_geom_v geom = geom1;
   geom.append(geom2);
   int ret = sort_merge_elems(geom, "vef", eps, equiv_elems, true);
   
   /*
   for(int i=0; i<3; i++) {
      const char *elems[] = { "verts", "edges", "faces" };
      fprintf(stderr, "\n%s\n", elems[i]);
      map<int, vector<int> >::iterator mi;
      for(mi=(*equiv_elems)[i].begin(); mi!=(*equiv_elems)[i].end(); ++mi) {
         fprintf(stderr, "%d <- ", mi->first);
         for(unsigned int j=0; j<mi->second.size(); j++)
            fprintf(stderr, "%d  ", mi->second[j]);
         fprintf(stderr, "\n");
      }
   }
   */
   
   return ret;
}
