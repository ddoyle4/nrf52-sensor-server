#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "BLE_DOMESTIC_SENSOR_SERVICE/BLE_DomesticSensorService.h"
#include "../mbed-os/targets/TARGET_NORDIC/TARGET_NRF5/TARGET_MCU_NRF52832/sdk/softdevice/s132/headers/nrf_soc.h"

const static char     DEVICE_NAME[] = "SSSPeripheral";
static const uint16_t uuid16_list[] = {0xFFFF};

/* Storage for current buffer sizes that are sent in 
ad packets - 4 bits each */
static const int gapBufferDataSize = 4;
static uint8_t * gapBufferData = (uint8_t *)malloc(sizeof(uint8_t)*gapBufferDataSize);

DigitalOut led1(LED1, 1);

static DomesticSensorService *DSServicePtr;

static EventQueue eventQueue(
    /* event count */ 32 * /* event size */ 32
);

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    BLE::Instance().gap().startAdvertising(); // restart advertising
}

void periodicCallback(void)
{
  led1 = !led1; /* Blinky, because why not? */
  
  //update buffer data for gap ad packet
  DSServicePtr->updateGapBufferData(gapBufferData);

  ble_error_t ret;
  ret=BLE::Instance().gap().updateAdvertisingPayload(GapAdvertisingData::MANUFACTURER_SPECIFIC_DATA, gapBufferData, gapBufferDataSize);
}

void onBleInitError(BLE &ble, ble_error_t error)
{
    (void)ble;
    (void)error;
   /* Initialization error handling should go here */
}

void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE&        ble   = params->ble;
    ble_error_t error = params->error;

    if (error != BLE_ERROR_NONE) {
        onBleInitError(ble, error);
        return;
    }

    if (ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);

    /* Setup primary service. */
    DSServicePtr = new DomesticSensorService(ble, &eventQueue);

    /* Setup advertising. */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(500); /* 1000ms */
    ble.gap().startAdvertising();

    sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
  BLE &ble = BLE::Instance();
  eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main()
{
  /*
   * NOTE: Not setting any particularly important time here. Just ensuring that
   * the clock is set to some value so that relative time measurements can be made
   * in the services used. 
   */
  Thread::attach_idle_hook(&sleep);

  set_time(1256729737);

  eventQueue.call_every(1000, periodicCallback);
  
  BLE &ble = BLE::Instance();

  ble.onEventsToProcess(scheduleBleEventsProcessing);
  ble.init(bleInitComplete);
  eventQueue.dispatch_forever();

  
  return 0;
}
