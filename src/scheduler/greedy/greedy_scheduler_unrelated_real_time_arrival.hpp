#pragma once

#include <vector>
#include <type_traits>
#include <cmath>
#include <queue>
#include <nlohmann/json.hpp>
#include <iostream>
#include <limits>

#include "basic_utils_in_one_header.hpp"

namespace SJF
{

using json = nlohmann::json;

/**
 * @brief Greedy scheduler of Identical machine_model, and real time arrival release model.
 * 
 * It's not optmized : currently, when we have to choose a "machine, job" pair, 
 * we just choose the job that is at the back of the acummulated jobs array
 * rather than use 2-layer loop to search for the best one
 */
class GreedySchedulerUnrelatedRealTimeArrival
{

public:
    GreedySchedulerUnrelatedRealTimeArrival()  {}

    /**
     * @brief Initialize the machine_free_list
     */
    void initialize(int64_t num_of_machines, const std::vector<UnrelatedMachine> & machines)
    {
        assert(num_of_machines == machines.size());
    }

    /**
     * @brief Schedule the jobs onto free machines and output schedule steps
     */
    std::vector<ScheduleStep> schedule(const std::vector<UnrelatedJob> & jobs_for_this_turn,
                                       std::vector<UnrelatedMachine> & machines,
                                       int64_t timestamp)
    {   
        NANO_LOG(DEBUG, "[GreedySchedulerUnrelatedRealTimeArrival::schedule] Inside schedule");

        // push the newly coming jobs into the job heap
        // (We have to consider the new jobs along with the accmulated jobs together)
        for(auto & job : jobs_for_this_turn)
        {
            accumulated_jobs_.push_back(job);

            // DEBUGING
            NANO_LOG(DEBUG, "%s", job.toString().c_str());
            NANO_LOG(DEBUG, "Jobs : ");
            for(auto & job_for_printing : accumulated_jobs_)
            {
                NANO_LOG(DEBUG, "%s", job_for_printing.toString().c_str());
            }
            NANO_LOG(DEBUG, "\n");
        }
        std::vector<ScheduleStep> schedule_steps;

        while(!accumulated_jobs_.empty())
        {
            // when there's free machine and remaining jobs, we can schedule the job at the top of the heap onto any machine
            const UnrelatedJob & current_job = accumulated_jobs_.back();

            int64_t num_free_machine = 0;
            int64_t min_executing_time = std::numeric_limits<int64_t>::max();
            int64_t target_machine_id = -1;

            for(size_t i = 0; i < machines.size(); i++)
            {
                auto & machine = machines[i];
                if(machine.isFree())
                {
                    num_free_machine++;
                    int64_t expected_executing_time = current_job.processing_time_[i];
                    if(expected_executing_time < min_executing_time)
                    {
                        min_executing_time = expected_executing_time;
                        target_machine_id = i;
                    }
                }
            }
            if(num_free_machine != 0)
            {
                assert((target_machine_id) != -1 && (min_executing_time != std::numeric_limits<int64_t>::max()));
                machines[target_machine_id].execute(current_job);
                schedule_steps.emplace_back(timestamp, current_job.id_, target_machine_id);

                NANO_LOG(DEBUG, "current_job_id is %ld, machine_id is %ld", current_job.id_, target_machine_id);
                NANO_LOG(DEBUG, "current job : %s", current_job.toString().c_str());
                accumulated_jobs_.pop_back();
            }
            if(num_free_machine <= 1)
            {
                break;
            }
        }

        NANO_LOG(DEBUG, "[GreedySchedulerUnrelatedRealTimeArrival::schedule] outside schedule");
        return schedule_steps;
    }

    /**
     * @brief Modify the remaining time of the busy machines and add the machines that has done their job
     * to the machine free list 
     */
    void updateMachineState(std::vector<UnrelatedMachine> & machines, int64_t elapsing_time)
    {
        NANO_LOG(DEBUG, "[GreedySchedulerUnrelatedRealTimeArrival::updateMachineState] Inside updateMachineState");
        NANO_LOG(DEBUG, "elapsing time : %ld", elapsing_time);
        NANO_LOG(DEBUG, "Printing the machines : ");
        for(auto & machine : machines)
        {
            NANO_LOG(DEBUG, "%s", machine.toString().c_str());
        }
        NANO_LOG(DEBUG, "\n");

        NANO_LOG(DEBUG, "Printing the accumulated_jobs_");
        NANO_LOG(DEBUG, "accumulated_jobs_.size() is %ld", accumulated_jobs_.size());
        for(auto & job : accumulated_jobs_)
        {
            NANO_LOG(DEBUG, "%s", job.toString().c_str());
        }
        NANO_LOG(DEBUG, "\n");

        bool done_flag = accumulated_jobs_.empty();
        for(size_t i = 0; i < machines.size(); i++)
        {
            auto & machine = machines[i];
            if(machine.remaining_time_ != Invalid_Remaining_Time)
            {
                done_flag = false;
                machine.remaining_time_ = std::max(static_cast<int64_t>(0), machine.remaining_time_ - elapsing_time);
                if(machine.remaining_time_ == 0)
                {
                    machine.setFree();
                }
            }
        }
        is_done_ = done_flag;
        // If there's no running job on any machine, set is_done_ to true

        NANO_LOG(DEBUG, "[GreedySchedulerUnrelatedRealTimeArrival::updateMachineState] Outside updateMachineState");
    }

    /**
     * @brief Check if there's no more jobs.
     * 
     * @return true is there's no more jobs, false otherwise.
     */
    bool done() const
    {
        return is_done_;
    }

private:
    /**
     * @brief The node for machine temp array. Includes machine id, total pending time and the corresponding processing speed.
     */
    struct MachineStateNode
    {
        int64_t machineId_;
        int64_t processing_speed_;

        MachineStateNode() = default;

        MachineStateNode(int64_t machineId, int64_t processing_speed) 
            : machineId_(machineId), processing_speed_(processing_speed) {}
        MachineStateNode(const MachineStateNode & other) = default;
        MachineStateNode(MachineStateNode && other) = default;
        MachineStateNode & operator=(const MachineStateNode & other) = default;
        MachineStateNode & operator=(MachineStateNode && other) = default;

        bool operator<(const MachineStateNode & other) const
        {
            return processing_speed_ < other.processing_speed_;
        }

        std::string toString() const
        { 
            std::stringstream ss;
            ss << "machineId : " << machineId_ << "\n";
            ss << "processing_speed : " << processing_speed_ << "\n";
            return ss.str();
        }
    };  

    std::vector<UnrelatedJob> accumulated_jobs_;    // a job heap storing all accumulated jobs1
    bool is_done_ = false;  
};

}
