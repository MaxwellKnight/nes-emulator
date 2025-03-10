class NESDebugger {
	constructor() {
		this.module = null;
		this.isLoaded = false;
		this.onLoadCallbacks = [];
		this.onBreakCallbacks = [];
		this.autoUpdateInterval = null;
	}

	onLoad(callback) {
		if (this.isLoaded) {
			callback();
		} else {
			this.onLoadCallbacks.push(callback);
		}
	}

	async init() {
		return new Promise((resolve, _) => {
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
		});
	}

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
		this.setPC = this.module.cwrap('debugger_set_pc', 'number', []);
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

		this._mainLoop = this.module.cwrap('main_loop', null, []);
	}

	onLoad(callback) {
		if (this.isLoaded) {
			callback();
		} else {
			this.onLoadCallbacks.push(callback);
		}
	}

	onBreak(callback) {
		this.onBreakCallbacks.push(callback);
	}

	startContinuousExecution(updateIntervalMs = 16) {
		if (!this.isLoaded) return;

		this.run();

		if (!this.hasBreakpointUIHandler) {
			this.onBreak(() => {
				this.stopContinuousExecution();

				const event = new CustomEvent('nes-breakpoint-hit', {
					detail: this.getState()
				});
				window.dispatchEvent(event);
			});
			this.hasBreakpointUIHandler = true;
		}

		const executionLoop = () => {
			if (!this.isRunning()) {
				this.onBreakCallbacks.forEach(callback => callback());
				cancelAnimationFrame(this.animationFrame);
				this.animationFrame = null;
				return;
			}

			const startTime = performance.now();
			const maxTimeSlice = 10; // Max 10ms per frame for execution
			let instructionsThisFrame = 0;

			while (this.isRunning() && performance.now() - startTime < maxTimeSlice) {
				this.step(); // Run one instruction
				instructionsThisFrame++;

				if (instructionsThisFrame > 1000) {
					break;
				}
			}

			this.animationFrame = requestAnimationFrame(executionLoop);
		};

		this.animationFrame = requestAnimationFrame(executionLoop);

		if (this.autoUpdateInterval) {
			clearInterval(this.autoUpdateInterval);
		}

		this.autoUpdateInterval = setInterval(() => {
			this.updateUI();
		}, updateIntervalMs);
	}

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

	stepInstruction() {
		if (!this.isLoaded) return;
		this.step();
		this.updateUI();
	}

	updateUI() {
		const event = new CustomEvent('nes-debugger-update', {
			detail: this.getState()
		});
		window.dispatchEvent(event);
	}

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

	disassembleAroundPC(instructionsBefore = 10, instructionsAfter = 20) {
		if (!this.isLoaded) return [];
		const rawString = this._disassembleAroundPC(instructionsBefore, instructionsAfter);

		if (!rawString) return [];

		const instructions = [];
		const regex = /(\d+)\|(\d+)\|([\w.]+)\|(\d+)\|([\w\s$#,()]+)\|(\d+)\|(\d+)/g;

		let match;
		while ((match = regex.exec(rawString)) !== null) {
			const instr = {
				address: parseInt(match[1], 10),
				opcode: parseInt(match[2], 10),
				mnemonic: match[3],
				operand: parseInt(match[4], 10),
				formatted: match[5],
				bytes: parseInt(match[6], 10),
				cycles: parseInt(match[7], 10)
			};

			instructions.push(instr);
		}

		// Fix up any entries that weren't matched by the regex
		// This is a brute force approach that searches for entries with specific opcodes
		// FIX: if not done manually then LDX, LDA are not parsed
		if (!instructions.some(i => i.opcode === 162)) { // LDX
			// Try to find LDX entries manually
			const ldxRegex = /(\d+)\|162\|LDX\|(\d+)\|.*?(\d+)\|(\d+)/g;
			while ((match = ldxRegex.exec(rawString)) !== null) {
				instructions.push({
					address: parseInt(match[1], 10),
					opcode: 162,
					mnemonic: "LDX",
					operand: parseInt(match[2], 10),
					formatted: `LDX #$${parseInt(match[2], 10).toString(16).toUpperCase().padStart(2, '0')}`,
					bytes: parseInt(match[3], 10) || 2,
					cycles: parseInt(match[4], 10) || 2
				});
			}
		}

		if (!instructions.some(i => i.opcode === 169)) { // LDA
			const ldaRegex = /(\d+)\|169\|LDA\|(\d+)\|.*?(\d+)\|(\d+)/g;
			while ((match = ldaRegex.exec(rawString)) !== null) {
				instructions.push({
					address: parseInt(match[1], 10),
					opcode: 169,
					mnemonic: "LDA",
					operand: parseInt(match[2], 10),
					formatted: `LDA #$${parseInt(match[2], 10).toString(16).toUpperCase().padStart(2, '0')}`,
					bytes: parseInt(match[3], 10) || 2,
					cycles: parseInt(match[4], 10) || 2
				});
			}
		}

		instructions.sort((a, b) => a.address - b.address);
		return instructions;
	}

	disassembleRange(startAddr, endAddr) {
		if (!this.isLoaded) return [];

		const str = this._disassembleRange(startAddr, endAddr);
		if (!str) return [];

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

	readMemoryRange(startAddr, endAddr) {
		if (!this.isLoaded) return [];

		const memory = [];
		for (let addr = startAddr; addr <= endAddr; addr++) {
			memory.push(this.readMemory(addr));
		}
		return memory;
	}

	loadBinary(data, startAddr = 0x0200) {
		if (!this.isLoaded) return;

		for (let i = 0; i < data.length; i++) {
			this.writeMemory(startAddr + i, data[i]);
		}
	}

	loadROM(data, startAddr = 0x0200) {
		this.loadBinary(data, startAddr);
		// Set reset vector to point to the start address
		this.writeMemory(0xFFFD, (startAddr >> 8) & 0xFF);
	}

	loadOpcodes(opcodes, startAddr = 0x0200) {
		console.log("loadOpcodes function", opcodes);
		if (!this.isLoaded) return;

		let opcodeArray;
		if (typeof opcodes === 'string') {
			opcodeArray = this.parseOpcodeString(opcodes);
		} else if (Array.isArray(opcodes)) {
			opcodeArray = opcodes;
		} else if (opcodes instanceof Uint8Array) {
			opcodeArray = Array.from(opcodes);
		} else {
			console.error('Invalid opcodes format. Expected string, array, or Uint8Array.');
			return;
		}

		this.loadBinary(opcodeArray, startAddr);
		this.writeMemory(0xFFFC, startAddr & 0xFF);
		this.writeMemory(0xFFFD, (startAddr >> 8) & 0xFF);
		this.reset();
	}

	parseOpcodeString(opcodeStr) {
		// Remove comments and any non-hex characters
		const cleanedStr = opcodeStr.replace(/;.*$/gm, '')  // Remove comments
			.replace(/[^0-9A-Fa-f\s,]/g, '')  // Remove non-hex, non-whitespace, non-comma chars
			.trim();

		// Split by whitespace or commas
		const hexValues = cleanedStr.split(/[\s,]+/);

		// Convert hex strings to numbers
		const opcodes = [];
		for (const hex of hexValues) {
			if (hex) {  // Skip empty strings
				const value = parseInt(hex, 16);
				if (!isNaN(value) && value >= 0 && value <= 255) {
					opcodes.push(value);
				} else {
					console.warn(`Skipping invalid opcode value: ${hex}`);
				}
			}
		}

		return opcodes;
	}

	loadExampleProgram(programName) {
		const examplePrograms = {
			'counter': 'A2 0A 8E 00 00 A2 03 8E 01 00 AC 00 00 A9 00 18 6D 01 00 88 D0 FA 8D 02 00 EA EA EA',
			'fibonacci': 'A9 01 85 00 A9 01 85 01 A5 00 18 65 01 85 02 A5 01 85 00 A5 02 85 01 A5 01 C9 55 90 EF 00',
			'loop': 'A2 0A CA 8E 00 02 E0 00 D0 F8 EA EA EA',
		};

		if (examplePrograms[programName]) {
			this.loadOpcodes(examplePrograms[programName]);
			return true;
		} else {
			console.error(`Example program "${programName}" not found.`);
			return false;
		}
	}
}

window.nesDebugger = new NESDebugger();
