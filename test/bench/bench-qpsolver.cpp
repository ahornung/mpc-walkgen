
#include "../src/common/mpc-debug.h"
#include <mpc-walkgen/humanoid/sharedpgtypes.h>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <cstring>

#include <fstream>

#include "../src/common/qp-solver.h"
#include <mpc-walkgen/common/qp-solver-type.h>
#include <memory>

using namespace Eigen;
using namespace MPCWalkgen;
using namespace Humanoid;
/*
  Test whole body problem
*/

bool testBenchmarkQP (const std::string& paths, QPSolver &qp,
                      unsigned fDofWb, unsigned fNconstraints)
{
  bool isSuccess = false;
  // nb unknow and constraint of the current problem
  qp.nbVar(6+fDofWb);
  qp.nbCtr(fNconstraints);

  std::fstream s_H((paths+"/wb_H").c_str(), std::fstream::in);
  std::fstream s_g((paths+"/wb_g").c_str(), std::fstream::in);
  std::fstream s_A((paths+"/wb_A").c_str(), std::fstream::in);
  std::fstream s_lb((paths+"/wb_lb").c_str(), std::fstream::in);
  std::fstream s_ub((paths+"/wb_ub").c_str(), std::fstream::in);
  std::fstream s_lbA((paths+"/wb_lbA").c_str(), std::fstream::in);
  std::fstream s_ubA((paths+"/wb_ubA").c_str(), std::fstream::in);
  std::fstream s_solution((paths+"/wb_solution").c_str(), std::fstream::in);

  if (
      !s_H.is_open() ||
      !s_g.is_open() ||
      !s_A.is_open() ||
      !s_lb.is_open() ||
      !s_ub.is_open() ||
      !s_lbA.is_open() ||
      !s_ubA.is_open() ||
      !s_solution.is_open()
      )
  {
    std::cout << "data not found" << std::endl;
    return isSuccess;
  }

  std::string line;

  // H
  std::vector<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > H_List;

  while (!s_H.eof())
  {
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> H = Eigen::MatrixXd::Zero(6+fDofWb, 6+fDofWb);
    getline(s_H, line);
    int pos=0;
    for (unsigned int i=0; i<fDofWb+6; i++)
    {
      for (unsigned int j=0; j<fDofWb+6; j++)
      {
        int pos2 = line.find(" ", pos);
        H(i, j) = atof(line.substr(pos, pos2-pos).c_str());
        pos=pos2+1;
      }
    }
    H_List.push_back(H);
  }

  // g
  std::vector<Eigen::VectorXd> g_List;

  while (!s_g.eof())
  {
    Eigen::VectorXd g = Eigen::VectorXd::Zero(6+fDofWb);
    getline(s_g, line);
    int pos=0;
    for (unsigned int i=0; i<fDofWb+6; i++)
    {
        int pos2 = line.find(" ", pos);
        g(i) = atof(line.substr(pos, pos2-pos).c_str());
        pos=pos2+1;
    }
    g_List.push_back(g);
  }

  // A
  std::vector<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > A_List;

  while (!s_A.eof())
  {
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> A = Eigen::MatrixXd::Zero(fNconstraints, 6+fDofWb);
    getline(s_A, line);
    int pos=0;
    for (unsigned int i=0; i<fNconstraints; i++)
    {
      for (unsigned int j=0; j<fDofWb+6; j++)
      {
        int pos2 = line.find(" ", pos);
        A(i, j) = atof(line.substr(pos, pos2-pos).c_str());
        pos=pos2+1;
      }
    }
    A_List.push_back(A);
  }

  // lb
  std::vector<Eigen::VectorXd> lb_List;

  while (!s_lb.eof())
  {
    Eigen::VectorXd lb = Eigen::VectorXd::Zero(6+fDofWb);
    getline(s_lb, line);
    int pos=0;
    for (unsigned int i=0; i<6+fDofWb; i++)
    {
        int pos2 = line.find(" ", pos);
        lb(i) = atof(line.substr(pos, pos2-pos).c_str());
        pos=pos2+1;
    }
    lb_List.push_back(lb);
  }

  // ub
  std::vector<Eigen::VectorXd> ub_List;

  while (!s_ub.eof())
  {
    Eigen::VectorXd ub = Eigen::VectorXd::Zero(6+fDofWb);
    getline(s_ub, line);
    int pos=0;
    for (unsigned int i=0; i<6+fDofWb; i++)
    {
        int pos2 = line.find(" ", pos);
        ub(i) = atof(line.substr(pos, pos2-pos).c_str());
        pos=pos2+1;
    }
    ub_List.push_back(ub);
  }

  // lbA
  std::vector<Eigen::VectorXd> lbA_List;

  while (!s_lbA.eof())
  {
    Eigen::VectorXd lbA = Eigen::VectorXd::Zero(fNconstraints);
    getline(s_lbA, line);
    int pos=0;
    for (unsigned int i=0; i<fNconstraints; i++)
    {
        int pos2 = line.find(" ", pos);
        lbA(i) = atof(line.substr(pos, pos2-pos).c_str());
        pos=pos2+1;
    }
    lbA_List.push_back(lbA);
  }

  // ubA
  std::vector<Eigen::VectorXd> ubA_List;

  while (!s_ubA.eof())
  {
    Eigen::VectorXd ubA = Eigen::VectorXd::Zero(fNconstraints);
    getline(s_ubA, line);
    int pos=0;
    for (unsigned int i=0; i<fNconstraints; i++)
    {
        int pos2 = line.find(" ", pos);
        ubA(i) = atof(line.substr(pos, pos2-pos).c_str());
        pos=pos2+1;
    }
    ubA_List.push_back(ubA);
  }

  // solution
  std::vector<Eigen::VectorXd> solution_List;

  while (!s_solution.eof())
  {
    Eigen::VectorXd solution = Eigen::VectorXd::Zero(6+fDofWb);
    getline(s_solution, line);
    int pos=0;
    for (unsigned int i=0; i<6+fDofWb; i++)
    {
        int pos2 = line.find(" ", pos);
        solution(i) = atof(line.substr(pos, pos2-pos).c_str());
        pos=pos2+1;
    }
    solution_List.push_back(solution);
  }

  if (
      H_List.size() != g_List.size() ||
      H_List.size() != A_List.size() ||
      H_List.size() != lb_List.size() ||
      H_List.size() != ub_List.size() ||
      H_List.size() != lbA_List.size() ||
      H_List.size() != ubA_List.size() ||
      H_List.size() != solution_List.size()
      )
  {
    std::cout << "vector not same size." << std::endl;
    return isSuccess;
  }

  MPCDebug debug(true);

  MPCSolution result;
  result.initialSolution.resize(6+fDofWb);
  result.initialConstraints.resize(fNconstraints+6+fDofWb);

  for (unsigned int i=0; i<H_List.size(); i++)
  {
    qp.reset();

    qp.matrix(matrixQ).addTerm(H_List.at(i));

    qp.vector(vectorP).addTerm(g_List.at(i));

    qp.matrix(matrixA).addTerm(A_List.at(i));

    qp.vector(vectorBL).addTerm(lbA_List.at(i));

    qp.vector(vectorBU).addTerm(ubA_List.at(i));

    qp.vector(vectorXL).addTerm(lb_List.at(i));

    qp.vector(vectorXU).addTerm(ub_List.at(i));

    result.reset();
    if (i==0)
    {
      result.useWarmStart = false;
    }
    else
    {
      result.useWarmStart = true;
      result.initialSolution = result.qpSolution;
      result.initialConstraints = result.constraints;
    }

    Eigen::VectorXd tmp(100);
    debug.getTime(1, true);
    qp.solve(result.qpSolution, result.constraints,
             result.initialSolution, result.initialConstraints,
             tmp, tmp,
             result.useWarmStart);
    debug.getTime(1, false);

    isSuccess = true;
    for (unsigned int j=0; j<6+fDofWb; j++)
    {
      if ((result.qpSolution(j)-solution_List.at(i)(j)) > 0.006 ||
          (result.qpSolution(j)-solution_List.at(i)(j)) < -0.006)
      {
        std::cout << "i: " << i
                  << " j: " << j
                  << " " << result.qpSolution(j)
                  << " " << solution_List.at(i)(j) << std::endl;
        isSuccess = false;
      }
    }
  }

  //std::cout << "100%" << std::endl;
  //std::cout << "bench-qpsolver test :" << std::endl;
  std::cout << "\tMean iteration duration: " << debug.computeInterval(1) << " us" << std::endl;
  std::cout << "\tsuccess " << isSuccess << std::endl;

  return isSuccess;
}



int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cout << "Usage:\n bench-qpsolver /path/to/data/dir" << std::endl;
    return 1;
  }
  std::string data_dir(argv[1]);
  unsigned int lDofWb = 23;
  unsigned int lNconstraints = 18;
  QPSolverType lSolverType;
#ifdef MPC_WALKGEN_WITH_QPOASES
  std::cout << "bench-qpsolver test qpOASES " << std::endl;
  lSolverType = QPSOLVERTYPE_QPOASES;
#endif

#ifdef MPC_WALKGEN_WITH_LSSOL
  std::cout << "bench-qpsolver test LSSOL " << std::endl;
  lSolverType = QPSOLVERTYPE_LSSOL;
#endif
  const std::auto_ptr<QPSolver> solver(
      createQPSolver(lSolverType, 6+lDofWb, lNconstraints));
  bool success = testBenchmarkQP(data_dir, *solver, lDofWb, lNconstraints);
  return (success ? 0 : 1);
}
