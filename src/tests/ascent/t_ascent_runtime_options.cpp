//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2015-2018, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-716457
//
// All rights reserved.
//
// This file is part of Ascent.
//
// For details, see: http://software.llnl.gov/ascent/.
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
/// file: t_ascent_runtime_options.cpp
///
//-----------------------------------------------------------------------------

#include "gtest/gtest.h"

#include <ascent.hpp>

#include <iostream>
#include <math.h>
#include <sstream>

#include <conduit_blueprint.hpp>

#include "t_config.hpp"
#include "t_utils.hpp"


using namespace std;
using namespace conduit;
using namespace ascent;


index_t EXAMPLE_MESH_SIDE_DIM = 20;
//-----------------------------------------------------------------------------
TEST(ascent_runtime_options, verbose_msgs)
{
    //
    // Create example mesh.
    //
    Node data, verify_info;
    conduit::blueprint::mesh::examples::braid("quads",100,100,0,data);

    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));
    verify_info.print();

    Node actions;
    Node &hello = actions.append();
    hello["action"]   = "hello!";
    actions.print();

    // we want the "empty" example pipeline
    Node open_opts;
    open_opts["runtime/type"] = "empty";
    open_opts["messages"] = "verbose";

    //
    // Run Ascent
    //
    Ascent ascent;
    ascent.open(open_opts);
    ascent.publish(data);
    ascent.execute(actions);
    ascent.close();
}

//-----------------------------------------------------------------------------
TEST(ascent_runtime_options, quiet_msgs)
{
    //
    // Create example mesh.
    //
    Node data, verify_info;
    conduit::blueprint::mesh::examples::braid("quads",100,100,0,data);

    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));
    verify_info.print();

    Node actions;
    Node &hello = actions.append();
    hello["action"]   = "hello!";
    actions.print();

    // we want the "empty" example pipeline
    Node open_opts;
    open_opts["runtime/type"] = "empty";
    open_opts["messages"] = "quiet";

    //
    // Run Ascent
    //
    Ascent ascent;
    ascent.open(open_opts);
    ascent.publish(data);
    ascent.execute(actions);
    ascent.close();
}


//-----------------------------------------------------------------------------
TEST(ascent_runtime_options, forward_exceptions)
{


    // invoke error by choosing bad runtime
    Node open_opts;
    open_opts["exceptions"] = "forward";
    open_opts["runtime/type"] = "BAD";


    //
    // Run Ascent
    //
    Ascent ascent;
    EXPECT_THROW(ascent.open(open_opts),conduit::Error);
    ascent.close();
}


//-----------------------------------------------------------------------------
TEST(ascent_runtime_options, catch_exceptions)
{


    // make sure bad runtime selection is caught
    Node open_opts;
    open_opts["exceptions"] = "catch";
    open_opts["runtime/type"] = "BAD";


    //
    // Run Ascent
    //
    Ascent ascent;
    ascent.open(open_opts);
    ascent.close();
}


//-----------------------------------------------------------------------------
TEST(ascent_runtime_options, test_errors)
{
    // invoke error cases caused by not initializing ascent

    Ascent ascent;
    Node n;
    // these will error to std::out, but we want to check they dont cras
    ascent.publish(n);
    ascent.publish(n);
    ascent.close();

    Node open_opts;
    open_opts["exceptions"] = "forward";
    ascent.open(open_opts);
    ascent.close();

    EXPECT_THROW(ascent.publish(n),conduit::Error);
    EXPECT_THROW(ascent.publish(n),conduit::Error);
    ascent.close();

}

//-----------------------------------------------------------------------------
TEST(ascent_runtime_options, test_actions_file)
{
    // the ascent runtime is currently our only rendering runtime
    Node n;
    ascent::about(n);
    // only run this test if ascent was built with vtkm support
    if(n["runtimes/ascent/vtkm/status"].as_string() == "disabled")
    {
        ASCENT_INFO("Ascent support disabled, skipping 3D default"
                      "Pipeline test");

        return;
    }


    //
    // Create an example mesh.
    //
    Node data, verify_info;
    conduit::blueprint::mesh::examples::braid("hexs",
                                              EXAMPLE_MESH_SIDE_DIM,
                                              EXAMPLE_MESH_SIDE_DIM,
                                              EXAMPLE_MESH_SIDE_DIM,
                                              data);

    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));
    //verify_info.print();

    ASCENT_INFO("Testing custom actions file");


    string output_path = prepare_output_dir();
    string output_file = conduit::utils::join_file_path(output_path,"tout_render_actions_img");
    string output_actions = conduit::utils::join_file_path(output_path,"tout_render_actions");

    // remove old images before rendering
    remove_test_image(output_file);
    remove_test_file(output_actions);


    //
    // Create the actions.
    //
    std::string actions_file = ""
                              "  [\n"
                              "    {\n"
                              "      \"action\": \"add_scenes\",\n"
                              "      \"scenes\": \n"
                              "      {\n"
                              "        \"s1\": \n"
                              "        {\n"
                              "          \"plots\":\n"
                              "          {\n"
                              "            \"p1\": \n"
                              "            {\n"
                              "              \"type\": \"pseudocolor\",\n"
                              "              \"field\": \"braid\"\n"
                              "            }\n"
                              "          },\n"
                              "          \"renders\": \n"
                              "          {\n"
                              "            \"r1\": \n"
                              "            {\n"
                              "              \"image_name\": \"" + output_file + "\"\n"
                              "            }\n"
                              "          }\n"
                              "        }\n"
                              "      }\n"
                              "    },\n"
                              "    {\n"
                              "      \"action\": \"execute\"\n"
                              "    }\n"
                              "  ]\n";

    std::ofstream file(output_actions);
    file<<actions_file;
    file.close();
    //
    // Run Ascent
    //

    Ascent ascent;

    Node ascent_opts;
    //ascent_opts["ascent_info"] = "verbose";
    ascent_opts["runtime/type"] = "ascent";
    ascent_opts["actions_file"] = output_actions;
    ascent.open(ascent_opts);
    ascent.publish(data);
    conduit::Node blank_actions;
    ascent.execute(blank_actions);
    ascent.close();
    // check that we created an image
    EXPECT_TRUE(check_test_image(output_file));
    EXPECT_TRUE(check_test_file(output_actions));
}
