class NESDebugger {
	constructor() {
		this.module = null;
		this.isLoaded = false;
		this.onLoadCallbacks = [];
		this.onBreakCallbacks = [];
		this.autoUpdateInterval = null;
	}

	// Initialize the debugger with the WASM module
	async init() {
		return new Promise((resolve, reject) => {
			// Set up the Module configuration
			window.Module = {
				onRuntimeInitialized: () => {
					this.module = window.Module;
					this.setupFunctions();
					this.isLoaded = true;
					this.onLoadCallbacks.forEach(callback => callback());
					resolve();
				},
				print: (text) => {
					console.log('WASM stdout:', text);
				},
				printErr: (text) => {
					console.error('WASM stderr:', text);
				}
			};

			// Load the WASM JavaScript glue code
			const script = document.createElement('script');
			script.src = 'nes_debugger.js';
			script.onerror = () => reject(new Error('Failed to load WASM module'));
			document.body.appendChild(script);
		});
	}

	// Set up function wrappers for the WASM exports
	setupFunctions() {
		// Execution control
		this.step = this.module.cwrap('debugger_step', null, []);
		this.run = this.module.cwrap('debugger_run', null, []);
		this.stop = this.module.cwrap('debugger_stop', null, []);
		this.reset = this.module.cwrap('debugger_reset', null, []);
		this.isRunning = this.module.cwrap('debugger_is_running', 'number', []);

		// Breakpoints
		this.addBreakpoint = this.module.cwrap('debugger_add_breakpoint', null, ['number']);
		this.removeBreakpoint = this.module.cwrap('debugger_remove_breakpoint', null, ['number']);
		this.clearBreakpoints = this.module.cwrap('debugger_clear_breakpoints', null, []);

		// Registers
		this.getRegisterA = this.module.cwrap('debugger_get_register_a', 'number', []);
		this.getRegisterX = this.module.cwrap('debugger_get_register_x', 'number', []);
		this.getRegisterY = this.module.cwrap('debugger_get_register_y', 'number', []);
		this.getRegisterSP = this.module.cwrap('debugger_get_register_sp', 'number', []);
		this.getRegisterPC = this.module.cwrap('debugger_get_register_pc', 'number', []);
		this.getRegisterStatus = this.module.cwrap('debugger_get_register_status', 'number', []);
		this.getStatusFlag = this.module.cwrap('debugger_get_status_flag', 'number', ['number']);

		// Memory
		this.readMemory = this.module.cwrap('debugger_read_memory', 'number', ['number']);
		this.writeMemory = this.module.cwrap('debugger_write_memory', null, ['number', 'number']);

		// Statistics
		this.getInstructionCount = this.module.cwrap('debugger_get_instruction_count', 'number', []);
		this.getCycleCount = this.module.cwrap('debugger_get_cycle_count', 'number', []);

		// Disassembly
		this._disassembleAroundPC = this.module.cwrap('debugger_disassemble_around_pc', 'string', ['number', 'number']);
		this._disassembleRange = this.module.cwrap('debugger_disassemble_range', 'string', ['number', 'number']);

		// Main loop
		this._mainLoop = this.module.cwrap('main_loop', null, []);
	}

	// Register a callback to be called when the debugger is loaded
	onLoad(callback) {
		if (this.isLoaded) {
			callback();
		} else {
			this.onLoadCallbacks.push(callback);
		}
	}

	// Register a callback to be called when the debugger hits a breakpoint
	onBreak(callback) {
		this.onBreakCallbacks.push(callback);
	}

	// Start continuous execution with regular UI updates
	startContinuousExecution(updateIntervalMs = 16) {
		if (!this.isLoaded) return;

		this.run();

		// Set up the execution loop
		const executionLoop = () => {
			if (!this.isRunning()) {
				// If we're not running, we hit a breakpoint or were stopped
				this.onBreakCallbacks.forEach(callback => callback());
				cancelAnimationFrame(this.animationFrame);
				this.animationFrame = null;
				return;
			}

			// Execute a batch of instructions
			const startTime = performance.now();
			const maxTimeSlice = 10; // Max 10ms per frame for execution

			while (this.isRunning() && performance.now() - startTime < maxTimeSlice) {
				this._mainLoop(); // Run one instruction
			}

			this.animationFrame = requestAnimationFrame(executionLoop);
		};

		// Start the execution loop
		this.animationFrame = requestAnimationFrame(executionLoop);

		// Set up auto-update for UI
		if (this.autoUpdateInterval) {
			clearInterval(this.autoUpdateInterval);
		}

		this.autoUpdateInterval = setInterval(() => {
			this.updateUI();
		}, updateIntervalMs);
	}

	// Stop continuous execution
	stopContinuousExecution() {
		if (this.animationFrame) {
			cancelAnimationFrame(this.animationFrame);
			this.animationFrame = null;
		}

		if (this.autoUpdateInterval) {
			clearInterval(this.autoUpdateInterval);
			this.autoUpdateInterval = null;
		}

		this.stop();
		this.updateUI();
	}

	// Execute a single instruction
	stepInstruction() {
		if (!this.isLoaded) return;
		this.step();
		this.updateUI();
	}

	// Update the UI with current state
	updateUI() {
		// This should be implemented by the UI layer
		// We'll dispatch a custom event that the UI can listen for
		const event = new CustomEvent('nes-debugger-update', {
			detail: this.getState()
		});
		window.dispatchEvent(event);
	}

	// Get the current state of the CPU/debugger
	getState() {
		if (!this.isLoaded) return null;

		return {
			registers: {
				A: this.getRegisterA(),
				X: this.getRegisterX(),
				Y: this.getRegisterY(),
				SP: this.getRegisterSP(),
				PC: this.getRegisterPC(),
				status: this.getRegisterStatus(),
				flags: {
					C: this.getStatusFlag(0), // CARRY
					Z: this.getStatusFlag(1), // ZERO
					I: this.getStatusFlag(2), // INTERRUPT_DISABLE
					D: this.getStatusFlag(3), // DECIMAL
					B: this.getStatusFlag(4), // BREAK
					U: this.getStatusFlag(5), // UNUSED
					V: this.getStatusFlag(6), // OVERFLOW
					N: this.getStatusFlag(7)  // NEGATIVE
				}
			},
			stats: {
				instructionCount: this.getInstructionCount(),
				cycleCount: this.getCycleCount()
			},
			running: this.isRunning() === 1
		};
	}

	// Get disassembly around the current PC
	disassembleAroundPC(instructionsBefore = 10, instructionsAfter = 20) {
		if (!this.isLoaded) return [];

		const str = this._disassembleAroundPC(instructionsBefore, instructionsAfter);
		if (!str) return [];

		// Parse the custom format: address|opcode|mnemonic|operand|formatted|bytes|cycles#address|...
		return str.split('#').map(item => {
			const parts = item.split('|');
			return {
				address: parseInt(parts[0], 10),
				opcode: parseInt(parts[1], 10),
				mnemonic: parts[2],
				operand: parseInt(parts[3], 10),
				formatted: parts[4],
				bytes: parseInt(parts[5], 10),
				cycles: parseInt(parts[6], 10)
			};
		});
	}

	// Get disassembly for a specific memory range
	disassembleRange(startAddr, endAddr) {
		if (!this.isLoaded) return [];

		const str = this._disassembleRange(startAddr, endAddr);
		if (!str) return [];

		// Parse the custom format: address|opcode|mnemonic|operand|formatted|bytes|cycles#address|...
		return str.split('#').map(item => {
			const parts = item.split('|');
			return {
				address: parseInt(parts[0], 10),
				opcode: parseInt(parts[1], 10),
				mnemonic: parts[2],
				operand: parseInt(parts[3], 10),
				formatted: parts[4],
				bytes: parseInt(parts[5], 10),
				cycles: parseInt(parts[6], 10)
			};
		});
	}

	// Read a range of memory
	readMemoryRange(startAddr, endAddr) {
		if (!this.isLoaded) return [];

		const memory = [];
		for (let addr = startAddr; addr <= endAddr; addr++) {
			memory.push(this.readMemory(addr));
		}
		return memory;
	}

	// Load a binary file into memory at the specified address
	loadBinary(data, startAddr) {
		if (!this.isLoaded) return;

		for (let i = 0; i < data.length; i++) {
			this.writeMemory(startAddr + i, data[i]);
		}
	}

	// Load a ROM file (simplified for this example)
	loadROM(data) {
		// In a real NES emulator, ROM loading would be more complex
		// This is a simplified version that just loads at a fixed address
		this.loadBinary(data, 0x8000);

		// Set reset vector to point to the start of ROM
		this.writeMemory(0xFFFC, 0x00);
		this.writeMemory(0xFFFD, 0x80);

		// Reset the CPU to start execution
		this.reset();
	}
}

// Create a global instance
window.nesDebugger = new NESDebugger();
