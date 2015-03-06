package main

import (
	"fmt"
)

type R3HealthAggregator struct{}

func (a *R3HealthAggregator) Aggregate(levels *[]Level) Level {

}

const (
	Red HealthLevel = iota, Yellow, Orange, Green
)

func main() {
	monitor, err := health.NewMonitor()
	if err != nil {
		// error handling
	}

	monitor.AddServices([]string{
		"timeslicer", "nav-stack", "ipc-logcat",
	})

	monitor.AddHealthLevels([]HealthLevel{
		Green, Orange, Yellow, Red,
	})

	monitor.SetReduceFunction(func(services []Service) HealthLevel {
		// collapse
	})

	monitor.SetLevelHandler(level, func(m *Monitor, l *HealthLevel) error {
		// run, do stuff
	})

	monitor.AddSubMonitor(subMonitor)

}

/*
 *  * How will the health monitor map incoming IPC health messages to their
 *    corresponding services?
 *    * by name?        <= all things points to this one
 *    * by identifier?
 *
 *  * How will the health monitor know what the ordering of health levels is?
 *    * Health levels are integers. Higher levels are healthier than lower ones.
 *
 *  * What will the health monitor's behavior be on disconnection?
 *
 *  * What will the health monitor's backoff policy be? Can this be configured?
 *
 *  * When and how often will the handler be run? Are there any restrictions
 *    on what the handler can and cannot do? Can a handler be long-running?
 */
