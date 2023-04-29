#include "_Plugin_Helper.h"
#ifdef USES_P147

// #######################################################################################################
// ############################ Plugin 147: Gases - SGP4x CO2 (VOC), NOx (SGP41) #########################
// #######################################################################################################

/**
 * 2023-04-29 tonhuisman: Implement sensor raw reading
 * 2023-04-27 tonhuisman: Start of plugin
 *
 * Using direct I2C functions and Sensirion VOC/NOx calculation library
 **/
# define PLUGIN_147
# define PLUGIN_ID_147          147
# define PLUGIN_NAME_147        "Gases - CO2 SGP4x"
# define PLUGIN_VALUENAME1_147  "VOC"
# define PLUGIN_VALUENAME2_147  "NOx"

# include "./src/PluginStructs/P147_data_struct.h"

boolean Plugin_147(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_147;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_147);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_147));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_147));

      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P147_SENSOR_TYPE == static_cast<int>(P147_sensor_e::SGP41) ? 2 : 1; // SGP41 also has NOx value
      success     = true;
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = event->Par1 == P147_I2C_ADDRESS;
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P147_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0; // VOC index value is an integer
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0; // NOx index value is an integer
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *sensorTypes[] = {
          F("SGP40"),
          F("SGP41"),
        };
        const int sensorTypeOptions[] = {
          static_cast<int>(P147_sensor_e::SGP40),
          static_cast<int>(P147_sensor_e::SGP41),
        };
        addFormSelector(F("Sensor model"), F("ptype"), 2, sensorTypes, sensorTypeOptions, P147_SENSOR_TYPE, true);

        // # ifndef LIMIT_BUILD_SIZE
        addFormNote(F("Page will reload on change."));

        // # endif // ifndef LIMIT_BUILD_SIZE
      }

      addFormSelector_YesNo(F("Use Calibration"), F("cal"), P147_GET_USE_CALIBRATION, true);

      // # ifndef LIMIT_BUILD_SIZE
      addFormNote(F("Page will reload on change."));

      // # endif // ifndef LIMIT_BUILD_SIZE

      if (P147_GET_USE_CALIBRATION) {
        addRowLabel(F("Temperature Task"));
        addTaskSelect(F("ttask"), P147_TEMPERATURE_TASK);

        if (validTaskIndex(P147_TEMPERATURE_TASK)) {
          addRowLabel(F("Temperature Value"));
          addTaskValueSelect(F("tvalue"), P147_TEMPERATURE_VALUE, P147_TEMPERATURE_TASK);
        }

        addRowLabel(F("Humidity Task"));
        addTaskSelect(F("htask"), P147_HUMIDITY_TASK);

        if (validTaskIndex(P147_HUMIDITY_TASK)) {
          addRowLabel(F("Humidity Value"));
          addTaskValueSelect(F("hvalue"), P147_HUMIDITY_VALUE, P147_HUMIDITY_TASK);
        }
      }

      addFormCheckBox(F("Low-power measurement"), F("plow"), P147_LOW_POWER_MEASURE == 1);
      addFormNote(F("Unchecked= 1 sec., continuous heating, Checked= 10 sec. measurement interval."));

      addFormSeparator(2);

      addFormCheckBox(F("Show raw data only"), F("raw"), P147_GET_RAW_DATA_ONLY == 1);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      int prevSensor = P147_SENSOR_TYPE;
      P147_SENSOR_TYPE       = getFormItemInt(F("ptype"));
      P147_LOW_POWER_MEASURE = isFormItemChecked(F("plow")) ? 1 : 0;
      P147_SET_USE_CALIBRATION(getFormItemInt(F("cal")));
      P147_SET_RAW_DATA_ONLY(isFormItemChecked(F("raw")) ? 1 : 0);

      if (P147_GET_USE_CALIBRATION) {
        P147_TEMPERATURE_TASK  = getFormItemInt(F("ttask"));
        P147_TEMPERATURE_VALUE = getFormItemInt(F("tvalue"));
        P147_HUMIDITY_TASK     = getFormItemInt(F("htask"));
        P147_HUMIDITY_VALUE    = getFormItemInt(F("hvalue"));
      }

      if ((prevSensor != P147_SENSOR_TYPE) && (P147_SENSOR_TYPE == static_cast<int>(P147_sensor_e::SGP41))) {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_147));
        ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0; // NOx index value is an integer
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P147_data_struct(event));
      P147_data_struct *P147_data = static_cast<P147_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P147_data) && P147_data->init(event);

      break;
    }

    case PLUGIN_TASKTIMER_IN:
    {
      P147_data_struct *P147_data = static_cast<P147_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P147_data) {
        success = P147_data->plugin_tasktimer_in(event);
      }

      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P147_data_struct *P147_data = static_cast<P147_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P147_data) {
        success = P147_data->plugin_once_a_second(event);
      }

      break;
    }

    case PLUGIN_READ:
    {
      P147_data_struct *P147_data = static_cast<P147_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P147_data) {
        success = P147_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P147_data_struct *P147_data = static_cast<P147_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P147_data) {
        success = P147_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P147_data_struct *P147_data = static_cast<P147_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P147_data) {
        success = P147_data->plugin_get_config_value(event, string);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P147
