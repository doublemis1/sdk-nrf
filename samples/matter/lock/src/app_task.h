/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "board/board.h"
#include "board/led_widget.h"
#include "bolt_lock_manager.h"
#include <app/clusters/door-lock-server/door-lock-delegate.h>

struct k_timer;
struct Identify;

#ifdef CONFIG_THREAD_WIFI_SWITCHING
enum class SwitchButtonAction : uint8_t { Pressed, Released };
#endif

#ifdef CONFIG_NCS_SAMPLE_MATTER_TEST_EVENT_TRIGGERS
#include "event_triggers/event_triggers.h"
#endif

using namespace ::chip;
static constexpr size_t DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE = 20;
static constexpr size_t DOOR_LOCK_CREDENTIAL_INFO_MAX_TYPES     = 8; // 0: ProgrammingPIN ~ 5: Face


class DelegateImpl : public chip::app::Clusters::DoorLock::Delegate {
public:
	uint8_t mAliroReaderVerificationKey[chip::app::Clusters::DoorLock::kAliroReaderVerificationKeySize];
	uint8_t mAliroReaderGroupIdentifier[chip::app::Clusters::DoorLock::kAliroReaderGroupIdentifierSize];
	uint8_t mAliroReaderGroupSubIdentifier[chip::app::Clusters::DoorLock::kAliroReaderGroupSubIdentifierSize];
	uint8_t mAliroGroupResolvingKey[chip::app::Clusters::DoorLock::kAliroGroupResolvingKeySize];
	bool mAliroStateInitialized = false;
	CHIP_ERROR GetAliroReaderVerificationKey(MutableByteSpan & verificationKey) override;
	CHIP_ERROR GetAliroReaderGroupIdentifier(MutableByteSpan & groupIdentifier) override;
	CHIP_ERROR GetAliroReaderGroupSubIdentifier(MutableByteSpan & groupSubIdentifier) override;
	CHIP_ERROR CopyProtocolVersionIntoSpan(uint16_t protocolVersionValue, MutableByteSpan & protocolVersion);
	CHIP_ERROR GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion) override;
	CHIP_ERROR GetAliroGroupResolvingKey(MutableByteSpan & groupResolvingKey) override;
	CHIP_ERROR GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion) override;
	uint8_t GetAliroBLEAdvertisingVersion() override;
	uint16_t GetNumberOfAliroCredentialIssuerKeysSupported() override;
	uint16_t GetNumberOfAliroEndpointKeysSupported() override;
	CHIP_ERROR SetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey,
							const ByteSpan & groupIdentifier, const Optional<ByteSpan> & groupResolvingKey) override;
	CHIP_ERROR ClearAliroReaderConfig() override;

};

class AppTask {
public:
	static AppTask &Instance()
	{
		static AppTask sAppTask;
		return sAppTask;
	};

	CHIP_ERROR StartApp();

	void UpdateClusterState(BoltLockManager::State state, BoltLockManager::OperationSource source);
	static void IdentifyStartHandler(Identify *);
	static void IdentifyStopHandler(Identify *);

private:
	CHIP_ERROR Init();

	static void LockActionEventHandler();
	static void ButtonEventHandler(Nrf::ButtonState state, Nrf::ButtonMask hasChanged);
	static void LockStateChanged(BoltLockManager::State state, BoltLockManager::OperationSource source);

#ifdef CONFIG_THREAD_WIFI_SWITCHING
	static void SwitchTransportEventHandler();
	static void SwitchTransportTimerTimeoutCallback(k_timer *timer);
	static void SwitchTransportTriggerHandler(const SwitchButtonAction &action);
#endif

#ifdef CONFIG_CHIP_NUS
	static void NUSLockCallback(void *context);
	static void NUSUnlockCallback(void *context);
#endif

#ifdef CONFIG_NCS_SAMPLE_MATTER_TEST_EVENT_TRIGGERS
	constexpr static Nrf::Matter::TestEventTrigger::EventTriggerId kDoorLockJammedEventTriggerId = 0xFFFF'FFFF'3277'4000;
	static CHIP_ERROR DoorLockJammedEventCallback(Nrf::Matter::TestEventTrigger::TriggerValue);
#endif
};
