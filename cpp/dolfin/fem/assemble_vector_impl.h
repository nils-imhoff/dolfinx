// Copyright (C) 2018-2019 Garth N. Wells
//
// This file is part of DOLFIN (https://www.fenicsproject.org)
//
// SPDX-License-Identifier:    LGPL-3.0-or-later

#pragma once

#include <Eigen/Dense>
#include <dolfin/common/types.h>
#include <memory>
#include <petscsys.h>
#include <vector>

namespace dolfin
{

namespace fem
{
class DirichletBC;
class Form;

namespace impl
{

/// Assemble linear form into an Eigen vector
void assemble(Eigen::Ref<Eigen::Matrix<PetscScalar, Eigen::Dynamic, 1>> b,
              const Form& L);

/// Modify b such that:
///
///   b <- b - scale * A_j (g_j - x0_j)
///
/// where j is a block (nest) index. For non-blocked probelem j = 1. The
/// boundary conditions bc1 are on the trial spaces V_j. The forms in
/// [a] must have the same test space as L (from b was built), but the
/// trial space may differ. If x0 is not supplied, then it is treated as
/// zero.
void apply_lifting(
    Eigen::Ref<Eigen::Matrix<PetscScalar, Eigen::Dynamic, 1>> b,
    const std::vector<std::shared_ptr<const Form>> a,
    std::vector<std::vector<std::shared_ptr<const DirichletBC>>> bcs1,
    std::vector<Eigen::Ref<const Eigen::Matrix<PetscScalar, Eigen::Dynamic, 1>>>
        x0,
    double scale);

/// Modify RHS vector to account for boundary condition b <- b - scale*Ax_bc
void lift_bc(
    Eigen::Ref<Eigen::Matrix<PetscScalar, Eigen::Dynamic, 1>> b, const Form& a,
    const Eigen::Ref<const Eigen::Matrix<PetscScalar, Eigen::Dynamic, 1>>
        bc_values1,
    const std::vector<bool>& bc_markers1, double scale);

/// Modify RHS vector to account for boundary condition such that b <- b
/// - scale*A (x_bc - x0)
void lift_bc(
    Eigen::Ref<Eigen::Matrix<PetscScalar, Eigen::Dynamic, 1>> b, const Form& a,
    const Eigen::Ref<const Eigen::Matrix<PetscScalar, Eigen::Dynamic, 1>>
        bc_values1,
    const std::vector<bool>& bc_markers1,
    Eigen::Ref<const Eigen::Matrix<PetscScalar, Eigen::Dynamic, 1>> x0,
    double scale);

} // namespace impl
} // namespace fem
} // namespace dolfin