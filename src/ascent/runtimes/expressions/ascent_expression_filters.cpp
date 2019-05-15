//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2015-2019, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-716457
//
// All rights reserved.
//
// This file is part of Ascent.
//
// For details, see: http://ascent.readthedocs.io/.
//
// Please also read ascent/LICENSE
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the disclaimer below.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the disclaimer (as noted below) in the
//   documentation and/or other materials provided with the distribution.
//
// * Neither the name of the LLNS/LLNL nor the names of its contributors may
//   be used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
// LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//


//-----------------------------------------------------------------------------
///
/// file: ascent_expression_filters.cpp
///
//-----------------------------------------------------------------------------

#include "ascent_expression_filters.hpp"

//-----------------------------------------------------------------------------
// thirdparty includes
//-----------------------------------------------------------------------------

// conduit includes
#include <conduit.hpp>

//-----------------------------------------------------------------------------
// ascent includes
//-----------------------------------------------------------------------------
#include <ascent_logging.hpp>
#include "ascent_conduit_reductions.hpp"
#include "ascent_blueprint_architect.hpp"
#include <flow_graph.hpp>
#include <flow_workspace.hpp>

#include <limits>
#include <math.h>

#ifdef ASCENT_MPI_ENABLED
#include <mpi.h>
#endif
using namespace conduit;
using namespace std;

using namespace flow;

//-----------------------------------------------------------------------------
// -- begin ascent:: --
//-----------------------------------------------------------------------------
namespace ascent
{

//-----------------------------------------------------------------------------
// -- begin ascent::runtime --
//-----------------------------------------------------------------------------
namespace runtime
{

//-----------------------------------------------------------------------------
// -- begin ascent::runtime::expressions--
//-----------------------------------------------------------------------------
namespace expressions
{

namespace detail
{

bool is_math(const std::string &op)
{
  if(op == "*" || op == "+" || op == "/" || op == "-")
  {
    return true;
  }
  else
  {
    return false;
  }
}

void
vector_op(const double lhs[3],
          const double rhs[3],
          const std::string &op,
          double res[3])
{

  if(op == "+")
  {
    res[0] = lhs[0] + rhs[0];
    res[1] = lhs[1] + rhs[1];
    res[2] = lhs[2] + rhs[2];
  }
  else if(op == "-")
  {
    res[0] = lhs[0] - rhs[0];
    res[1] = lhs[1] - rhs[1];
    res[2] = lhs[2] - rhs[2];
  }
  else
  {
    ASCENT_ERROR("Unsupported vector op "<<op);
  }
}


template<typename T>
T math_op(const T& lhs, const T& rhs, const std::string &op)
{
  T res;
  if(op == "+")
  {
    res = lhs + rhs;
  }
  else if(op == "-")
  {
    res = lhs - rhs;
  }
  else if(op == "*")
  {
    res = lhs * rhs;
  }
  else if(op == "/")
  {
    res = lhs / rhs;
  }
  else
  {
    ASCENT_ERROR("unknow math op "<<op);
  }
  return res;
}

template<typename T>
int comp_op(const T& lhs, const T& rhs, const std::string &op)
{
  int res;
  if(op == "<")
  {
    res = lhs < rhs;
  }
  else if(op == "<=")
  {
    res = lhs <= rhs;
  }
  else if(op == ">")
  {
    res = lhs > rhs;
  }
  else if(op == ">=")
  {
    res = lhs >= rhs;
  }
  else if(op == "==")
  {
    res = lhs == rhs;
  }
  else
  {
    ASCENT_ERROR("unknow comparison op "<<op);
  }
  return res;
}

} // namespace detail

//-----------------------------------------------------------------------------
NullArg::NullArg()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
NullArg::~NullArg()
{
// empty
}

//-----------------------------------------------------------------------------
void
NullArg::declare_interface(Node &i)
{
    i["type_name"]   = "null_arg";
    i["port_names"] = DataType::empty();
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
NullArg::verify_params(const conduit::Node &params,
                       conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
NullArg::execute()
{
   conduit::Node *output = new conduit::Node();
   set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
Identifier::Identifier()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
Identifier::~Identifier()
{
// empty
}

//-----------------------------------------------------------------------------
void
Identifier::declare_interface(Node &i)
{
    i["type_name"]   = "expr_identifier";
    i["port_names"] = DataType::empty();
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
Identifier::verify_params(const conduit::Node &params,
                          conduit::Node &info)
{
    info.reset();
    bool res = true;
    if(!params.has_path("value"))
    {
       info["errors"].append() = "Missing required string parameter 'value'";
       res = false;
    }
    return res;
}


//-----------------------------------------------------------------------------
void
Identifier::execute()
{

   conduit::Node *output = new conduit::Node();
   std::string i_name = params()["value"].as_string();

   conduit::Node *cache = graph().workspace().registry().fetch<Node>("cache");
   if(!cache->has_path(i_name))
   {
     ASCENT_ERROR("Unknown expression identifier: '"<<i_name<<"'");
   }

   const int entries = (*cache)[i_name].number_of_children();
   if(entries < 1)
   {
     ASCENT_ERROR("Expression identifier: needs a non-zero number of entires: "<<entries);
   }
   // grab the last one calculated
   (*output) = (*cache)[i_name].child(entries - 1);
   set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
Integer::Integer()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
Integer::~Integer()
{
// empty
}

//-----------------------------------------------------------------------------
void
Integer::declare_interface(Node &i)
{
    i["type_name"]   = "expr_integer";
    i["port_names"] = DataType::empty();
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
Integer::verify_params(const conduit::Node &params,
                       conduit::Node &info)
{
    info.reset();
    bool res = true;
    if(!params.has_path("value"))
    {
       info["errors"].append() = "Missing required numeric parameter 'value'";
       res = false;
    }
    return res;
}


//-----------------------------------------------------------------------------
void
Integer::execute()
{

   conduit::Node *output = new conduit::Node();
   (*output)["value"] = params()["value"].to_int32();
   (*output)["type"] = "scalar";
   set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
Double::Double()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
Double::~Double()
{
// empty
}

//-----------------------------------------------------------------------------
void
Double::declare_interface(Node &i)
{
    i["type_name"]   = "expr_double";
    i["port_names"] = DataType::empty();
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
Double::verify_params(const conduit::Node &params,
                      conduit::Node &info)
{
    info.reset();
    bool res = true;
    if(!params.has_path("value"))
    {
       info["errors"].append() = "Missing required numeric parameter 'value'";
       res = false;
    }
    return res;
}


//-----------------------------------------------------------------------------
void
Double::execute()
{

   conduit::Node *output = new conduit::Node();
   (*output)["value"] = params()["value"].to_float64();
   (*output)["type"] = "scalar";
   set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
MeshVar::MeshVar()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
MeshVar::~MeshVar()
{
// empty
}

//-----------------------------------------------------------------------------
void
MeshVar::declare_interface(Node &i)
{
    i["type_name"]   = "expr_meshvar";
    i["port_names"] = DataType::empty();
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
MeshVar::verify_params(const conduit::Node &params,
                       conduit::Node &info)
{
    info.reset();
    bool res = true;
    if(!params.has_path("value"))
    {
       info["errors"].append() = "Missing required string parameter 'value'";
       res = false;
    }
    return res;
}

//-----------------------------------------------------------------------------
void
MeshVar::execute()
{
   conduit::Node *output = new conduit::Node();

   (*output)["value"] = params()["value"].as_string();
   (*output)["type"] = "meshvar";
   set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
BinaryOp::BinaryOp()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
BinaryOp::~BinaryOp()
{
// empty
}

//-----------------------------------------------------------------------------
void
BinaryOp::declare_interface(Node &i)
{
    i["type_name"]   = "expr_binary_op";
    i["port_names"].append() = "lhs";
    i["port_names"].append() = "rhs";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
BinaryOp::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    if(!params.has_path("op_string"))
    {
       info["errors"].append() = "Missing required string parameter 'op_string'";
       res = false;
    }
    return res;
}


//-----------------------------------------------------------------------------
void
BinaryOp::execute()
{

  Node *n_lhs = input<Node>("lhs");
  Node *n_rhs = input<Node>("rhs");

  const Node &lhs = (*n_lhs)["value"];
  const Node &rhs = (*n_rhs)["value"];

  bool lhs_is_vector = false;
  bool rhs_is_vector = false;

  if((*n_lhs)["type"].as_string() == "vector")
  {
    lhs_is_vector = true;
  }

  if((*n_rhs)["type"].as_string() == "vector")
  {
    rhs_is_vector = true;
  }

  if((rhs_is_vector && !lhs_is_vector) ||
     (!rhs_is_vector && lhs_is_vector))
  {
    ASCENT_ERROR("Mixed vector and scalar quantities not implemeneted / supported");
  }

  std::string op = params()["op_string"].as_string();
  bool is_math = detail::is_math(op);
  conduit::Node *output = new conduit::Node();

  if(rhs_is_vector && lhs_is_vector)
  {
    if(!is_math)
    {
      ASCENT_ERROR("Boolean operators on vectors no allowed");
    }

    double res[3];
    detail::vector_op((*n_lhs)["value"].value(),
                      (*n_rhs)["value"].value(),
                      op,
                      res);


    (*output)["value"].set(res, 3);
    (*output)["type"] = "vector";
  }
  else
  {
    bool has_float = false;

    if(lhs.dtype().is_floating_point() ||
       rhs.dtype().is_floating_point())
    {
      has_float = true;
    }


    // promote to double if at one is a double

    if(has_float)
    {
      double d_rhs = rhs.to_float64();
      double d_lhs = lhs.to_float64();
      if(is_math)
      {
        (*output)["value"] = detail::math_op(d_lhs, d_rhs, op);
        (*output)["type"] = "scalar";
      }
      else
      {
        (*output)["value"] = detail::comp_op(d_lhs, d_rhs, op);
        (*output)["type"] = "boolean";
      }
    }
    else
    {
      int i_rhs = rhs.to_int32();
      int i_lhs = lhs.to_int32();
      if(is_math)
      {
        (*output)["value"] = detail::math_op(i_lhs, i_rhs, op);
        (*output)["type"] = "scalar";
      }
      else
      {
        (*output)["value"] = detail::comp_op(i_lhs, i_rhs, op);
        (*output)["type"] = "boolean";
      }
    }
  }

  //std::cout<<" operation "<<op<<"\n";

  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
ScalarMin::ScalarMin()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
ScalarMin::~ScalarMin()
{
// empty
}

//-----------------------------------------------------------------------------
void
ScalarMin::declare_interface(Node &i)
{
    i["type_name"]   = "scalar_min";
    i["port_names"].append() = "arg1";
    i["port_names"].append() = "arg2";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ScalarMin::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
ScalarMin::execute()

{

  Node *arg1 = input<Node>("arg1");
  Node *arg2 = input<Node>("arg2");


  bool has_float = false;

  if((*arg1)["value"].dtype().is_floating_point() ||
     (*arg2)["value"].dtype().is_floating_point())
  {
    has_float = true;
  }

  conduit::Node *output = new conduit::Node();

  if(has_float)
  {
    double d_rhs = (*arg1)["value"].to_float64();
    double d_lhs = (*arg2)["value"].to_float64();
    (*output)["value"] = std::min(d_lhs, d_rhs);
  }
  else
  {
    int i_rhs = (*arg1)["value"].to_int32();
    int i_lhs = (*arg2)["value"].to_int32();
    (*output)["value"] = std::min(i_lhs, i_rhs);
  }

  (*output)["type"] = "scalar";
  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
ScalarMax::ScalarMax()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
ScalarMax::~ScalarMax()
{
// empty
}

//-----------------------------------------------------------------------------
void
ScalarMax::declare_interface(Node &i)
{
    i["type_name"]   = "scalar_max";
    i["port_names"].append() = "arg1";
    i["port_names"].append() = "arg2";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ScalarMax::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
ScalarMax::execute()

{

  Node *arg1 = input<Node>("arg1");
  Node *arg2 = input<Node>("arg2");

  bool has_float = false;

  if((*arg1)["value"].dtype().is_floating_point() ||
     (*arg2)["value"].dtype().is_floating_point())
  {
    has_float = true;
  }

  conduit::Node *output = new conduit::Node();

  if(has_float)
  {
    double d_rhs = (*arg1)["value"].to_float64();
    double d_lhs = (*arg2)["value"].to_float64();
    (*output)["value"] = std::max(d_lhs, d_rhs);
  }
  else
  {
    int i_rhs = (*arg1)["value"].to_int32();
    int i_lhs = (*arg2)["value"].to_int32();
    (*output)["value"] = std::max(i_lhs, i_rhs);
  }

  (*output)["type"] = "scalar";
  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
FieldMin::FieldMin()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
FieldMin::~FieldMin()
{
// empty
}

//-----------------------------------------------------------------------------
void
FieldMin::declare_interface(Node &i)
{
    i["type_name"]   = "field_min";
    i["port_names"].append() = "arg1";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
FieldMin::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
FieldMin::execute()

{

  Node *arg1 = input<Node>("arg1");

  const std::string field = (*arg1)["value"].as_string();

  conduit::Node *output = new conduit::Node();

  if(!graph().workspace().registry().has_entry("dataset"))
  {
    ASCENT_ERROR("FieldMin: Missing dataset");
  }

  conduit::Node *dataset = graph().workspace().registry().fetch<Node>("dataset");

  if(!has_field(*dataset, field))
  {
    std::vector<std::string> names = dataset->child(0)["fields"].child_names();
    std::stringstream ss;
    ss<<"[";
    for(int i = 0; i < names.size(); ++i)
    {
      ss<<" "<<names[i];
    }
    ss<<"]";
    ASCENT_ERROR("FieldMin: dataset does not contain field '"<<field<<"'"
                 <<" known = "<<ss.str());
  }

  if(!is_scalar_field(*dataset, field))
  {
    ASCENT_ERROR("FieldMin: field '"<<field<<"' is not a scalar");
  }

  conduit::Node n_min = field_min(*dataset, field);

  (*output)["value"] = n_min["value"];
  (*output)["type"] = "scalar";
  (*output)["atts/position"] = n_min["position"];

  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
FieldMax::FieldMax()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
FieldMax::~FieldMax()
{
// empty
}

//-----------------------------------------------------------------------------
void
FieldMax::declare_interface(Node &i)
{
    i["type_name"]   = "field_max";
    i["port_names"].append() = "arg1";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
FieldMax::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
FieldMax::execute()

{

  Node *arg1 = input<Node>("arg1");

  const std::string field = (*arg1)["value"].as_string();

  conduit::Node *output = new conduit::Node();

  if(!graph().workspace().registry().has_entry("dataset"))
  {
    ASCENT_ERROR("FieldMax: Missing dataset");
  }

  conduit::Node *dataset = graph().workspace().registry().fetch<Node>("dataset");

  if(!has_field(*dataset, field))
  {
    std::vector<std::string> names = dataset->child(0)["fields"].child_names();
    std::stringstream ss;
    ss<<"[";
    for(int i = 0; i < names.size(); ++i)
    {
      ss<<" "<<names[i];
    }
    ss<<"]";
    ASCENT_ERROR("FieldMax: dataset does not contain field '"<<field<<"'"
                 <<" known = "<<ss.str());
  }

  if(!is_scalar_field(*dataset, field))
  {
    ASCENT_ERROR("FieldMax: field '"<<field<<"' is not a scalar");
  }

  conduit::Node n_max = field_max(*dataset, field);

  (*output)["value"] = n_max["value"];
  (*output)["type"] = "scalar";
  (*output)["atts/position"] = n_max["position"];

  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
FieldAvg::FieldAvg()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
FieldAvg::~FieldAvg()
{
// empty
}

//-----------------------------------------------------------------------------
void
FieldAvg::declare_interface(Node &i)
{
    i["type_name"]   = "field_avg";
    i["port_names"].append() = "arg1";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
FieldAvg::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
FieldAvg::execute()

{

  Node *arg1 = input<Node>("arg1");

  const std::string field = (*arg1)["value"].as_string();

  conduit::Node *output = new conduit::Node();

  if(!graph().workspace().registry().has_entry("dataset"))
  {
    ASCENT_ERROR("FieldAvg: Missing dataset");
  }

  conduit::Node *dataset = graph().workspace().registry().fetch<Node>("dataset");

  if(!has_field(*dataset, field))
  {
    std::vector<std::string> names = dataset->child(0)["fields"].child_names();
    std::stringstream ss;
    ss<<"[";
    for(int i = 0; i < names.size(); ++i)
    {
      ss<<" "<<names[i];
    }
    ss<<"]";
    ASCENT_ERROR("FieldAvg: dataset does not contain field '"<<field<<"'"
                 <<" known = "<<ss.str());
  }

  if(!is_scalar_field(*dataset, field))
  {
    ASCENT_ERROR("FieldAvg: field '"<<field<<"' is not a scalar");
  }

  conduit::Node n_avg = field_avg(*dataset, field);

  (*output)["value"] = n_avg["value"];
  (*output)["type"] = "scalar";

  set_output<conduit::Node>(output);
}


//-----------------------------------------------------------------------------
Position::Position()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
Position::~Position()
{
// empty
}

//-----------------------------------------------------------------------------
void
Position::declare_interface(Node &i)
{
    i["type_name"]   = "expr_position";
    i["port_names"].append() = "arg1";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
Position::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
Position::execute()
{

  Node *n_in = input<Node>("arg1");

  if(!n_in->has_path("atts/position"))
  {
    ASCENT_ERROR("Position: input does not have 'position' attribute");
  }

  conduit::Node *output = new conduit::Node();

  (*output)["value"] = (*n_in)["atts/position"];
  (*output)["type"] = "vector";

  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
Cycle::Cycle()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
Cycle::~Cycle()
{
// empty
}

//-----------------------------------------------------------------------------
void
Cycle::declare_interface(Node &i)
{
    i["type_name"]   = "cycle";
    i["port_names"] = DataType::empty();
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
Cycle::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
Cycle::execute()

{
  conduit::Node *output = new conduit::Node();

  if(!graph().workspace().registry().has_entry("dataset"))
  {
    ASCENT_ERROR("Cycle: Missing dataset");
  }

  conduit::Node *dataset = graph().workspace().registry().fetch<Node>("dataset");

  conduit::Node state = get_state_var(*dataset, "cycle");
  if(!state.dtype().is_number())
  {
    ASCENT_ERROR("Expressions: cycle() is not a number");
  }

  (*output)["type"] = "scalar";
  (*output)["value"] = state;
  set_output<conduit::Node>(output);
}
//-----------------------------------------------------------------------------
Vector::Vector()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
Vector::~Vector()
{
// empty
}

//-----------------------------------------------------------------------------
void
Vector::declare_interface(Node &i)
{
    i["type_name"]   = "vector";
    i["port_names"].append() = "arg1";
    i["port_names"].append() = "arg2";
    i["port_names"].append() = "arg3";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
Vector::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
Vector::execute()

{

  Node *arg1 = input<Node>("arg1");
  Node *arg2 = input<Node>("arg2");
  Node *arg3 = input<Node>("arg3");

  if( (*arg1)["type"].as_string() != "scalar" ||
      (*arg2)["type"].as_string() != "scalar" ||
      (*arg3)["type"].as_string() != "scalar")
  {
    ASCENT_ERROR("All components to vector constructor must be scalars");
  }

  double vec[3] = {0., 0., 0.};
  vec[0] = (*arg1)["value"].to_float64();
  vec[1] = (*arg2)["value"].to_float64();
  vec[2] = (*arg3)["value"].to_float64();

  conduit::Node *output = new conduit::Node();
  (*output)["type"] = "vector";
  (*output)["value"].set(vec,3);;
  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
Magnitude::Magnitude()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
Magnitude::~Magnitude()
{
// empty
}

//-----------------------------------------------------------------------------
void
Magnitude::declare_interface(Node &i)
{
    i["type_name"]   = "magnitude";
    i["port_names"].append() = "arg1";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
Magnitude::verify_params(const conduit::Node &params,
                         conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
Magnitude::execute()

{

  Node *arg1 = input<Node>("arg1");

  if( (*arg1)["type"].as_string() != "vector")
  {
    ASCENT_ERROR("Magnitude input must be a vector");
  }

  double res = 0.;
  const double *vec = (*arg1)["value"].value();
  res = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
  conduit::Node *output = new conduit::Node();
  (*output)["type"] = "scalar";
  (*output)["value"] = res;
  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
Histogram::Histogram()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
Histogram::~Histogram()
{
// empty
}

//-----------------------------------------------------------------------------
void
Histogram::declare_interface(Node &i)
{
    i["type_name"]   = "histogram";
    i["port_names"].append() = "arg1";
    i["port_names"].append() = "num_bins";
    i["port_names"].append() = "min_val";
    i["port_names"].append() = "max_val";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
Histogram::verify_params(const conduit::Node &params,
                         conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}

//-----------------------------------------------------------------------------
void
Histogram::execute()

{

  Node *arg1 = input<Node>("arg1");
  // optional inputs
  const Node *n_bins = input<Node>("num_bins");
  const Node *n_max = input<Node>("max_val");
  const Node *n_min = input<Node>("min_val");

  if( (*arg1)["type"].as_string() != "meshvar")
  {
    ASCENT_ERROR("Histogram: input must be a meshvar");
  }

  const std::string field = (*arg1)["value"].as_string();

  conduit::Node *dataset = graph().workspace().registry().fetch<Node>("dataset");

  if(!has_field(*dataset, field))
  {
    std::vector<std::string> names = dataset->child(0)["fields"].child_names();
    std::stringstream ss;
    ss<<"[";
    for(int i = 0; i < names.size(); ++i)
    {
      ss<<" "<<names[i];
    }
    ss<<"]";
    ASCENT_ERROR("Histogram: dataset does not contain field '"<<field<<"'"
                 <<" known = "<<ss.str());
  }

  int num_bins = 256;
  if(!n_bins->dtype().is_empty())
  {
    num_bins = (*n_bins)["value"].to_int32();
  }

  double min_val;
  double max_val;

  // handle the optional inputs
  if(!n_max->dtype().is_empty())
  {
    max_val = (*n_max)["value"].to_float64();
  }
  else
  {
    max_val = field_max(*dataset, field)["value"].to_float64();
  }

  if(!n_min->dtype().is_empty())
  {
    min_val = (*n_min)["value"].to_float64();
  }
  else
  {
    min_val = field_min(*dataset, field)["value"].to_float64();
  }

  if(min_val >=  max_val)
  {
    ASCENT_ERROR("Histogram: min value ("<<min_val<<") must be smaller than max ("<<max_val<<")");
  }

  conduit::Node *output = new conduit::Node();
  (*output)["value"] = field_histogram(*dataset, field, min_val, max_val, num_bins)["value"];
  (*output)["type"] = "histogram";
  (*output)["min_val"] = min_val;
  (*output)["max_val"] = max_val;
  (*output)["num_bins"] = num_bins;
  set_output<conduit::Node>(output);
}

};
//-----------------------------------------------------------------------------
// -- end ascent::runtime::expressions--
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end ascent::runtime --
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end ascent:: --
//-----------------------------------------------------------------------------





