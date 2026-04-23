/*
************************************************************************
*
*   mat_vec.h - utilities for matrices and vectors
*
*   Copyright (c) 1994
*
*   ETH Zuerich
*   Institut fuer Molekularbiologie und Biophysik
*   ETH-Hoenggerberg
*   CH-8093 Zuerich
*
*   SPECTROSPIN AG
*   Industriestr. 26
*   CH-8117 Faellanden
*
*   All Rights Reserved
*
*   Date of last modification : 95/01/12
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.mat_vec.h
*   SCCS identification       : 1.3
*
************************************************************************
*/
#ifndef _MAT_VEC_H_
#define _MAT_VEC_H_

typedef float Vec3[3];
typedef float Vec4[4];

typedef float Mat3[3][3];
typedef float Mat4[4][4];

extern void Vec3Zero(Vec3);
extern void Vec4Zero(Vec4);

extern void Vec3Copy(Vec3, Vec3);
extern void Vec4Copy(Vec4, Vec4);
extern void Vec3To4(Vec4, Vec3);
extern void Vec4To3(Vec3, Vec4);

extern float Vec3Abs(Vec3);
extern float Vec3DiffAbs(Vec3, Vec3);
extern float Vec4Abs(Vec3);
extern float Vec4DiffAbs(Vec3, Vec3);
extern void Vec3Norm(Vec3);
extern void Vec4Norm(Vec4);

extern void Vec3Scale(Vec3, float);
extern void Vec3Add(Vec3, Vec3);
extern void Vec3Sub(Vec3, Vec3);
extern void Vec3ScaleAdd(Vec3, float, Vec3);
extern void Vec3ScaleSub(Vec3, float, Vec3);
extern void Vec4Scale(Vec4, float);
extern void Vec4Add(Vec4, Vec4);
extern void Vec4Sub(Vec4, Vec4);
extern void Vec4ScaleAdd(Vec4, float, Vec4);
extern void Vec4ScaleSub(Vec4, float, Vec4);

extern float Vec3Scalar(Vec3, Vec3);
extern float Vec4Scalar(Vec4, Vec4);

extern void Vec3Cross(Vec3, Vec3);
extern void Vec4Cross(Vec4, Vec4);

extern void Mat3Ident(Mat3);
extern void Mat4Ident(Mat4);

extern void Mat3Copy(Mat3, Mat3);
extern void Mat4Copy(Mat4, Mat4);
extern void Mat3To4(Mat4, Mat3);

extern void Mat3Mult(Mat3, Mat3);
extern void Mat4Mult(Mat4, Mat4);

extern void Mat3VecMult(Vec3, Mat3);
extern void Mat4VecMult(Vec4, Mat4);

extern void Mat3Transp(Mat3);
extern void Mat4Transp(Mat4);

extern void Mat3Ortho(Mat3);
extern void Mat4Ortho(Mat4);

extern void Mat3Inv(Mat3);

extern void Mat3GetAng(Mat3, float *, float *, float *);
extern void Mat4GetAng(Mat4, float *, float *, float *);

#endif  /* _MAT_VEC_H_ */
