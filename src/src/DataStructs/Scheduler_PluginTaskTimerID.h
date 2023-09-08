#ifndef DATASTRUCTS_SCHEDULER_PLUGINTASKTIMERID_H
#define DATASTRUCTS_SCHEDULER_PLUGINTASKTIMERID_H

#include "../DataStructs/SchedulerTimerID.h"

#include "../DataTypes/TaskIndex.h"

/*********************************************************************************************\
* Plugin Task Timer  (PLUGIN_TASKTIMER_IN)
* Can be scheduled per combo taskIndex & Par1 (20 least significant bits)
\*********************************************************************************************/
struct PluginTaskTimerID : SchedulerTimerID {
  // taskIndex and par1 form a unique key that can be used to restart a timer
  PluginTaskTimerID(taskIndex_t taskIndex,
                    int         Par1);

  taskIndex_t getTaskIndex() const;

#ifndef BUILD_NO_DEBUG
  String      decode() const override;
#endif // ifndef BUILD_NO_DEBUG
};


#endif // ifndef DATASTRUCTS_SCHEDULER_PLUGINTASKTIMERID_H
