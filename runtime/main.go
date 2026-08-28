package main

import (
	"context"
	"fmt"
	"os"
	"os/signal"
	"syscall"

	"github.com/JBailes/aimee/server-go/bus"
	handler "github.com/JBailes/aimee/server-go/modules/memory"
)

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintf(os.Stderr, "usage: %s DAEMON_MODULE_BUS_SOCKET\n", os.Args[0])
		os.Exit(2)
	}
	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()
	config := bus.ModuleProcessConfig{
		SocketPath: os.Args[1], ModuleName: "memory",
		PrincipalClass: 1, PrincipalRef: 7,
		Stages: []bus.ModuleStage{
		{EventKind: 5889, StageID: 1},
		{EventKind: 5890, StageID: 2},
		{EventKind: 5891, StageID: 3},
		{EventKind: 5892, StageID: 4},
		{EventKind: 5893, StageID: 5},
		{EventKind: 5894, StageID: 6},
		},
		Handler: handler.Handle,
	}
	if err := bus.RunModuleProcess(ctx, config); err != nil {
		fmt.Fprintf(os.Stderr, "aimee-module-memory: %v\n", err)
		os.Exit(1)
	}
}
