#include <gtest/gtest.h>
#include <fstream>
#include <nlohmann/json.hpp>

#include "basic_utils_in_one_header.hpp"
#include "json_input_handler.hpp"

using namespace SJF;

using json = nlohmann::json;
json config;

TEST(JsonInputHandlerTest, ConstructorAndGetJobs1) 
{
    NanoLog::setLogFile("./ConstructorAndGetJobs1.log");
    NanoLog::setLogLevel(DEBUG);
    JsonInputHandler<Machine_Model::Identical> handler(config, "../../../../assets/json/job/normal_job1.json");

    // Test assert method
    EXPECT_TRUE(handler.checkValidity(2));
    EXPECT_THROW(handler.checkValidity(3), NumberOfMachinesMismatch);

    // Test getJobs method
    auto jobs = handler.getJobs(0);
    EXPECT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs.at(0).timestamp_, 0);
    EXPECT_EQ(jobs.at(0).workload_, 2);

    jobs = handler.getJobs(1);
    EXPECT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs.at(0).timestamp_, 1);
    EXPECT_EQ(jobs.at(0).workload_, 2);

    // Test done method
    EXPECT_TRUE(handler.done());
}

TEST(JsonInputHandlerTest, ConstructorAndGetJobs2) 
{
    NanoLog::setLogFile("./ConstructorAndGetJobs2.log");
    NanoLog::setLogLevel(DEBUG);
    JsonInputHandler<Machine_Model::Related> handler(config, "../../../../assets/json/job/normal_job2.json");

    NANO_LOG(DEBUG, "[JsonInputHandlerTest::ConstructorAndGetJobs2], printing the job array");
    auto job_array = handler.getJobArray();
    for(auto & job : job_array)
    {
        NANO_LOG(DEBUG, "%s", job.toString().c_str());
    }

    // Test assert method
    EXPECT_TRUE(handler.checkValidity(3));

    // Test getJobs method
    auto jobs = handler.getJobs(0);
    EXPECT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs.at(0).timestamp_, 0);
    EXPECT_EQ(jobs.at(0).workload_, 3);
    EXPECT_FALSE(handler.done());

    jobs = handler.getJobs(1);
    EXPECT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs.at(0).timestamp_, 1);
    EXPECT_EQ(jobs.at(0).workload_, 2);
    EXPECT_FALSE(handler.done());

    jobs = handler.getJobs(3);
    EXPECT_EQ(jobs.size(), 3);
    EXPECT_EQ(jobs.at(0).timestamp_, 2);
    EXPECT_EQ(jobs.at(0).workload_, 5);
    EXPECT_EQ(jobs.at(1).timestamp_, 2);
    EXPECT_EQ(jobs.at(1).workload_, 3);
    EXPECT_EQ(jobs.at(2).timestamp_, 3);
    EXPECT_EQ(jobs.at(2).workload_, 3);

    // Test done method
    EXPECT_TRUE(handler.done());
}

TEST(JsonInputHandlerTest, UnrelatedConstructorAndGetJobs1) 
{
    NanoLog::setLogFile("./UnrelatedConstructorAndGetJobs1.log");
    NanoLog::setLogLevel(DEBUG);
    JsonInputHandler<Machine_Model::Unrelated> handler(config, "../../../../assets/json/job/unrelated_job1.json");

    NANO_LOG(DEBUG, "[JsonInputHandlerTest::UnrelatedConstructorAndGetJobs1], printing the job array");
    auto job_array = handler.getJobArray();
    for(auto & job : job_array)
    {
        NANO_LOG(DEBUG, "%s", job.toString().c_str());
    }

    // Test assert method
    EXPECT_TRUE(handler.checkValidity(2));

    // Test getJobs method
    auto jobs = handler.getJobs(0);
    EXPECT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs.at(0).timestamp_, 0);
    EXPECT_EQ(jobs.at(0).processing_time_.size(), 2);
    EXPECT_EQ(jobs.at(0).processing_time_.at(0), 3);
    EXPECT_EQ(jobs.at(0).processing_time_.at(1), 2);
    EXPECT_FALSE(handler.done());

    jobs = handler.getJobs(3);
    EXPECT_EQ(jobs.size(), 2);
    EXPECT_EQ(jobs.at(0).timestamp_, 1);
    EXPECT_EQ(jobs.at(0).processing_time_.size(), 2);
    EXPECT_EQ(jobs.at(0).processing_time_.at(0), 5);
    EXPECT_EQ(jobs.at(0).processing_time_.at(1), 4);

    EXPECT_EQ(jobs.at(1).timestamp_, 2);
    EXPECT_EQ(jobs.at(1).processing_time_.size(), 2);
    EXPECT_EQ(jobs.at(1).processing_time_.at(0), 1);
    EXPECT_EQ(jobs.at(1).processing_time_.at(1), 2);

    // Test done method
    EXPECT_TRUE(handler.done());
}
