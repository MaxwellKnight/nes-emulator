class DebuggerUI {
	constructor() {
		this.debugger = window.nesDebugger;
		this.breakpoints = new Set();
		this.currentMemoryPage = 0x0000;
		this.memoryPageSize = 0x100;

		this.debugger.onLoad(() => {
			this.setupEventListeners();
			this.updateUI();
			console.log('NES Debugger loaded and ready');
		});

		this.debugger.init().catch(err => {
			console.error('Failed to initialize debugger:', err);
		});
	}

	setupEventListeners() {
		document.getElementById('resetButton').addEventListener('click', () => this.reset());
		document.getElementById('runButton').addEventListener('click', () => this.run());
		document.getElementById('stepButton').addEventListener('click', () => this.step());
		document.getElementById('stopButton').addEventListener('click', () => this.stop());

		document.getElementById('addBreakpointButton').addEventListener('click', () => this.addBreakpoint());

		document.getElementById('jumpToAddressButton').addEventListener('click', () => this.jumpToMemoryAddress());
		document.getElementById('memoryPage').addEventListener('change', () => this.changeMemoryPage());

		document.getElementById('loadRomButton').addEventListener('click', () => {
			document.getElementById('romFile').click();
		});

		document.getElementById('romFile').addEventListener('change', (e) => {
			this.loadROMFile(e.target.files[0]);
		});

		window.addEventListener('nes-debugger-update', () => this.updateUI());

		document.getElementById('disassemblyView').addEventListener('click', (e) => {
			const row = e.target.closest('.instruction');
			if (row) {
				const addr = parseInt(row.dataset.address, 16);
				this.toggleBreakpoint(addr);
			}
		});
	}

	reset() {
		this.debugger.reset();
		this.updateUI();
	}

	run() {
		this.debugger.startContinuousExecution();
	}

	step() {
		this.debugger.stepInstruction();
	}

	stop() {
		this.debugger.stopContinuousExecution();
	}

	addBreakpoint() {
		const input = document.getElementById('breakpointAddress');
		const addrStr = input.value.trim();

		if (addrStr) {
			try {
				const addr = parseInt(addrStr, 16);
				if (!isNaN(addr) && addr >= 0 && addr <= 0xFFFF) {
					this.debugger.addBreakpoint(addr);
					this.breakpoints.add(addr);
					input.value = '';
					this.updateBreakpointsList();
				}
			} catch (e) {
				console.error('Invalid breakpoint address:', e);
			}
		}
	}

	removeBreakpoint(addr) {
		this.debugger.removeBreakpoint(addr);
		this.breakpoints.delete(addr);
		this.updateBreakpointsList();
		this.updateDisassembly();
	}

	toggleBreakpoint(addr) {
		if (this.breakpoints.has(addr)) {
			this.removeBreakpoint(addr);
		} else {
			this.debugger.addBreakpoint(addr);
			this.breakpoints.add(addr);
			this.updateBreakpointsList();
			this.updateDisassembly();
		}
	}

	updateBreakpointsList() {
		const list = document.getElementById('breakpointsList');
		list.innerHTML = '';

		if (this.breakpoints.size === 0) {
			list.innerHTML = '<div style="color: #666;">No breakpoints set</div>';
			return;
		}

		const sortedBreakpoints = Array.from(this.breakpoints).sort((a, b) => a - b);

		for (const bp of sortedBreakpoints) {
			const item = document.createElement('div');
			item.style.display = 'flex';
			item.style.justifyContent = 'space-between';
			item.style.marginBottom = '5px';

			const addrSpan = document.createElement('span');
			addrSpan.textContent = `$${bp.toString(16).toUpperCase().padStart(4, '0')}`;

			const removeButton = document.createElement('button');
			removeButton.textContent = 'X';
			removeButton.style.padding = '0 5px';
			removeButton.addEventListener('click', () => this.removeBreakpoint(bp));

			item.appendChild(addrSpan);
			item.appendChild(removeButton);
			list.appendChild(item);
		}
	}

	jumpToMemoryAddress() {
		const input = document.getElementById('memoryAddress');
		const addrStr = input.value.trim();

		if (addrStr) {
			try {
				const addr = parseInt(addrStr, 16);
				if (!isNaN(addr) && addr >= 0 && addr <= 0xFFFF) {
					this.currentMemoryPage = addr & 0xFF00;
					this.updateMemoryView();
				}
			} catch (e) {
				console.error('Invalid memory address:', e);
			}
		}
	}

	changeMemoryPage() {
		const select = document.getElementById('memoryPage');
		const value = select.value;

		switch (value) {
			case 'zeropage':
				this.currentMemoryPage = 0x0000;
				break;
			case 'stack':
				this.currentMemoryPage = 0x0100;
				break;
			case 'ram':
				this.currentMemoryPage = 0x0200;
				break;
			case 'vectors':
				this.currentMemoryPage = 0xFFFA;
				this.memoryPageSize = 6; // Just show the 6 bytes of vectors
				break;
			default:
				this.currentMemoryPage = 0x0000;
		}

		this.updateMemoryView();
	}

	loadROMFile(file) {
		if (!file) return;

		const reader = new FileReader();
		reader.onload = (e) => {
			const data = new Uint8Array(e.target.result);
			this.debugger.loadROM(data);
			this.updateUI();
		};
		reader.readAsArrayBuffer(file);
	}

	// UI update functions
	updateUI() {
		this.updateRegisters();
		this.updateFlags();
		this.updateDisassembly();
		this.updateMemoryView();
		this.updateStats();
	}

	updateRegisters() {
		const state = this.debugger.getState();
		if (!state) return;

		const regs = state.registers;

		document.getElementById('reg-a').textContent = `$${regs.A.toString(16).toUpperCase().padStart(2, '0')}`;
		document.getElementById('reg-x').textContent = `$${regs.X.toString(16).toUpperCase().padStart(2, '0')}`;
		document.getElementById('reg-y').textContent = `$${regs.Y.toString(16).toUpperCase().padStart(2, '0')}`;
		document.getElementById('reg-sp').textContent = `$${regs.SP.toString(16).toUpperCase().padStart(2, '0')}`;
		document.getElementById('reg-pc').textContent = `$${regs.PC.toString(16).toUpperCase().padStart(4, '0')}`;
	}

	updateFlags() {
		const state = this.debugger.getState();
		if (!state) return;

		const flags = state.registers.flags;

		document.getElementById('flag-n').className = flags.N ? 'flag active' : 'flag inactive';
		document.getElementById('flag-v').className = flags.V ? 'flag active' : 'flag inactive';
		document.getElementById('flag-u').className = flags.U ? 'flag active' : 'flag inactive';
		document.getElementById('flag-b').className = flags.B ? 'flag active' : 'flag inactive';
		document.getElementById('flag-d').className = flags.D ? 'flag active' : 'flag inactive';
		document.getElementById('flag-i').className = flags.I ? 'flag active' : 'flag inactive';
		document.getElementById('flag-z').className = flags.Z ? 'flag active' : 'flag inactive';
		document.getElementById('flag-c').className = flags.C ? 'flag active' : 'flag inactive';
	}

	updateDisassembly() {
		const view = document.getElementById('disassemblyView');
		view.innerHTML = '';

		const instructions = this.debugger.disassembleAroundPC(10, 20);
		const pc = this.debugger.getRegisterPC();

		for (const instr of instructions) {
			const row = document.createElement('div');
			row.className = 'instruction';
			row.dataset.address = instr.address.toString(16);

			if (instr.address === pc) {
				row.className += ' current';
			}

			if (this.breakpoints.has(instr.address)) {
				row.style.borderLeft = '3px solid var(--warning-color)';
			}

			const addrSpan = document.createElement('span');
			addrSpan.textContent = `$${instr.address.toString(16).toUpperCase().padStart(4, '0')}`;

			const opcodeSpan = document.createElement('span');
			opcodeSpan.textContent = `${instr.opcode.toString(16).toUpperCase().padStart(2, '0')}`;

			const mnemonicSpan = document.createElement('span');
			mnemonicSpan.textContent = instr.mnemonic;

			const operandsSpan = document.createElement('span');
			operandsSpan.textContent = instr.formatted.substring(instr.mnemonic.length);

			row.appendChild(addrSpan);
			row.appendChild(opcodeSpan);
			row.appendChild(mnemonicSpan);
			row.appendChild(operandsSpan);

			view.appendChild(row);
		}
	}

	updateMemoryView() {
		const view = document.getElementById('memoryView');
		view.innerHTML = '';

		// Add headers
		const headerRow = document.createElement('div');
		headerRow.className = 'memory-row';

		const addrHeader = document.createElement('div');
		addrHeader.className = 'memory-header';
		addrHeader.textContent = 'Address';

		headerRow.appendChild(addrHeader);

		for (let i = 0; i < 16; i++) {
			const header = document.createElement('div');
			header.className = 'memory-header';
			header.textContent = i.toString(16).toUpperCase();
			headerRow.appendChild(header);
		}

		const asciiHeader = document.createElement('div');
		asciiHeader.className = 'memory-header';
		asciiHeader.textContent = 'ASCII';

		headerRow.appendChild(asciiHeader);
		view.appendChild(headerRow);

		// Calculate how many rows to display
		const rowCount = Math.min(16, Math.ceil(this.memoryPageSize / 16));

		// Add memory rows
		for (let row = 0; row < rowCount; row++) {
			const rowStartAddr = this.currentMemoryPage + (row * 16);

			const memoryRow = document.createElement('div');
			memoryRow.className = 'memory-row';

			const addrCell = document.createElement('div');
			addrCell.className = 'memory-address';
			addrCell.textContent = `$${rowStartAddr.toString(16).toUpperCase().padStart(4, '0')}`;

			memoryRow.appendChild(addrCell);

			let asciiText = '';

			for (let col = 0; col < 16; col++) {
				const addr = rowStartAddr + col;

				if (addr <= 0xFFFF) {
					const value = this.debugger.readMemory(addr);

					const cell = document.createElement('div');
					cell.className = 'memory-cell';
					cell.textContent = value.toString(16).toUpperCase().padStart(2, '0');

					// Highlight current PC
					if (addr === this.debugger.getRegisterPC()) {
						cell.style.backgroundColor = 'rgba(231, 76, 60, 0.3)';
					}

					// Highlight SP for stack memory page
					if (this.currentMemoryPage === 0x0100 && addr === 0x0100 + this.debugger.getRegisterSP()) {
						cell.style.backgroundColor = 'rgba(46, 204, 113, 0.3)';
					}

					memoryRow.appendChild(cell);

					// ASCII representation
					asciiText += (value >= 32 && value <= 126) ? String.fromCharCode(value) : '.';
				} else {
					const cell = document.createElement('div');
					cell.className = 'memory-cell';
					cell.textContent = '--';
					memoryRow.appendChild(cell);

					asciiText += ' ';
				}
			}

			const asciiCell = document.createElement('div');
			asciiCell.className = 'memory-ascii';
			asciiCell.textContent = asciiText;

			memoryRow.appendChild(asciiCell);
			view.appendChild(memoryRow);
		}
	}

	updateStats() {
		const state = this.debugger.getState();
		if (!state) return;

		document.getElementById('instructionCount').textContent = state.stats.instructionCount.toString();
		document.getElementById('cycleCount').textContent = state.stats.cycleCount.toString();

		const isRunning = state.running;
		document.getElementById('runButton').disabled = isRunning;
		document.getElementById('stopButton').disabled = !isRunning;
	}
}

document.addEventListener('DOMContentLoaded', () => {
	window.debuggerUI = new DebuggerUI();
});
