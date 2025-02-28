class DebuggerUI {
	constructor() {
		this.debugger = window.nesDebugger;
		this.breakpoints = new Set();
		this.currentMemoryPage = 0x0000;
		this.memoryPageSize = 0x100;
		this.theme = 'light';

		// Check if the debugger is already loaded
		if (this.debugger && this.debugger.isLoaded) {
			console.log('Debugger already loaded, initializing immediately');
			this.updateUI();
			console.log('NES Debugger loaded and ready');
			this.testMemoryAccess();
		} else if (this.debugger && typeof this.debugger.onLoad === 'function') {
			// If not loaded, register for the onLoad callback
			this.debugger.onLoad(() => {
				console.log('Debugger onLoad callback triggered');
				this.updateUI();
				console.log('NES Debugger loaded and ready');
				this.testMemoryAccess();
			});
		} else {
			console.error('Debugger not available or missing onLoad method');
		}
		this.setupEventListeners();
	}

	testMemoryAccess() {
		// Test memory write and read directly
		try {
			console.log("Testing memory read/write directly:");
			this.debugger.writeMemory(0x8000, 0xA9); // LDA immediate
			this.debugger.writeMemory(0x8001, 0x42); // #$42
			console.log("Memory at $8000 after direct write:",
				"0x" + this.debugger.readMemory(0x8000).toString(16).toUpperCase());
			console.log("Memory at $8001 after direct write:",
				"0x" + this.debugger.readMemory(0x8001).toString(16).toUpperCase());
		} catch (error) {
			console.error('Memory access test failed:', error);
		}
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

		// Add event listener for the opcode textarea and load button
		document.getElementById('loadOpcodesButton').addEventListener('click', () => {
			this.loadOpcodesFromText();
		});

		// Add event listener for theme toggle
		document.getElementById('themeToggle').addEventListener('click', () => this.toggleTheme());

		window.addEventListener('nes-debugger-update', () => this.updateUI());

		document.getElementById('disassemblyView').addEventListener('click', (e) => {
			const row = e.target.closest('.instruction');
			if (row) {
				const addr = parseInt(row.dataset.address, 16);
				this.toggleBreakpoint(addr);
			}
		});

		// Memory cell click event for editing values
		document.getElementById('memoryView').addEventListener('click', (e) => {
			const cell = e.target.closest('.memory-cell');
			if (cell && cell.dataset.address) {
				this.promptEditMemoryValue(parseInt(cell.dataset.address, 16));
			}
		});

		// Initialize tooltips
		this.initTooltips();
	}

	initTooltips() {
		const tooltipTriggerList = document.querySelectorAll('[data-bs-toggle="tooltip"]');
		[...tooltipTriggerList].forEach(tooltipTriggerEl => {
			new bootstrap.Tooltip(tooltipTriggerEl);
		});
	}

	toggleTheme() {
		const htmlElement = document.documentElement;
		const themeToggle = document.getElementById('themeToggle');

		if (htmlElement.getAttribute('data-bs-theme') === 'dark') {
			htmlElement.setAttribute('data-bs-theme', 'light');
			themeToggle.innerHTML = '<i class="bi bi-sun-fill me-2"></i><span>Light Mode</span>';
			this.theme = 'light';
		} else {
			htmlElement.setAttribute('data-bs-theme', 'dark');
			themeToggle.innerHTML = '<i class="bi bi-moon-stars-fill me-2"></i><span>Dark Mode</span>';
			this.theme = 'dark';
		}
	}

	reset() {
		this.debugger.reset();
		this.updateUI();
		this.showToast('CPU reset successfully', 'success');
	}

	run() {
		this.debugger.startContinuousExecution();
		this.showToast('Execution started', 'info');
	}

	step() {
		this.debugger.stepInstruction();
		this.showToast('Executed one instruction', 'info');
	}

	stop() {
		this.debugger.stopContinuousExecution();
		this.showToast('Execution stopped', 'warning');
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
					this.updateDisassembly();
					this.showToast(`Breakpoint added at $${addr.toString(16).toUpperCase().padStart(4, '0')}`, 'success');
				} else {
					this.showToast('Invalid address range', 'danger');
				}
			} catch (e) {
				console.error('Invalid breakpoint address:', e);
				this.showToast('Invalid address format', 'danger');
			}
		}
	}

	removeBreakpoint(addr) {
		this.debugger.removeBreakpoint(addr);
		this.breakpoints.delete(addr);
		this.updateBreakpointsList();
		this.updateDisassembly();
		this.showToast(`Breakpoint removed from $${addr.toString(16).toUpperCase().padStart(4, '0')}`, 'info');
	}

	toggleBreakpoint(addr) {
		if (this.breakpoints.has(addr)) {
			this.removeBreakpoint(addr);
		} else {
			this.debugger.addBreakpoint(addr);
			this.breakpoints.add(addr);
			this.updateBreakpointsList();
			this.updateDisassembly();
			this.showToast(`Breakpoint toggled at $${addr.toString(16).toUpperCase().padStart(4, '0')}`, 'success');
		}
	}

	updateBreakpointsList() {
		const list = document.getElementById('breakpointsList');
		list.innerHTML = '';

		if (this.breakpoints.size === 0) {
			list.innerHTML = '<div class="text-muted small">No breakpoints set</div>';
			return;
		}

		const sortedBreakpoints = Array.from(this.breakpoints).sort((a, b) => a - b);

		for (const bp of sortedBreakpoints) {
			const item = document.createElement('div');
			item.className = 'd-flex justify-content-between align-items-center mb-2';

			const addrSpan = document.createElement('span');
			addrSpan.className = 'badge bg-light text-dark';
			addrSpan.textContent = `$${bp.toString(16).toUpperCase().padStart(4, '0')}`;

			const removeButton = document.createElement('button');
			removeButton.innerHTML = '<i class="bi bi-x-circle"></i>';
			removeButton.className = 'btn btn-sm btn-outline-danger';
			removeButton.setAttribute('data-bs-toggle', 'tooltip');
			removeButton.setAttribute('data-bs-placement', 'left');
			removeButton.setAttribute('title', 'Remove Breakpoint');
			removeButton.addEventListener('click', () => this.removeBreakpoint(bp));

			item.appendChild(addrSpan);
			item.appendChild(removeButton);
			list.appendChild(item);
		}

		// Re-initialize tooltips for the new buttons
		this.initTooltips();
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
					this.showToast(`Jumped to memory address $${addr.toString(16).toUpperCase().padStart(4, '0')}`, 'info');
				} else {
					this.showToast('Invalid address range', 'danger');
				}
			} catch (e) {
				console.error('Invalid memory address:', e);
				this.showToast('Invalid address format', 'danger');
			}
		}
	}

	changeMemoryPage() {
		const select = document.getElementById('memoryPage');
		const value = select.value;

		switch (value) {
			case 'zeropage':
				this.currentMemoryPage = 0x0000;
				this.memoryPageSize = 0x100;
				break;
			case 'stack':
				this.currentMemoryPage = 0x0100;
				this.memoryPageSize = 0x100;
				break;
			case 'ram':
				this.currentMemoryPage = 0x0200;
				this.memoryPageSize = 0x100;
				break;
			case 'vectors':
				this.currentMemoryPage = 0xFFFA;
				this.memoryPageSize = 6; // Just show the 6 bytes of vectors
				break;
			default:
				this.currentMemoryPage = 0x0000;
				this.memoryPageSize = 0x100;
		}

		this.updateMemoryView();
		this.showToast(`Memory view changed to ${select.options[select.selectedIndex].text}`, 'info');
	}

	loadROMFile(file) {
		if (!file) return;

		const reader = new FileReader();
		reader.onload = (e) => {
			const data = new Uint8Array(e.target.result);
			this.debugger.loadROM(data);
			this.updateUI();
			this.showToast(`ROM loaded: ${file.name} (${data.length} bytes)`, 'success');
		};
		reader.readAsArrayBuffer(file);
	}

	// Load opcodes from text input
	loadOpcodesFromText() {
		const textArea = document.getElementById('opcodeInput');
		const opcodesText = textArea.value.trim();

		if (!opcodesText) {
			this.showToast('Please enter opcodes', 'warning');
			return;
		}

		try {
			// Parse the input text containing hex values
			const opcodes = this.parseOpcodes(opcodesText);

			if (opcodes.length === 0) {
				this.showToast('No valid opcodes found', 'warning');
				return;
			}

			// Create Uint8Array from the parsed opcodes
			const data = new Uint8Array(opcodes);

			// Load opcodes into memory
			this.debugger.loadROM(data);

			// Update the UI to reflect the changes
			this.updateUI();

			// Show success message
			this.showToast(`Loaded ${data.length} bytes successfully`, 'success');

		} catch (e) {
			console.error('Error loading opcodes:', e);
			this.showToast(`Error: ${e.message}`, 'danger');
		}
	}

	// Parse opcode text into array of numbers
	parseOpcodes(text) {
		// Remove comments (anything after a semicolon on a line)
		text = text.replace(/;.*$/gm, '');

		// Remove all whitespace and split by any whitespace or commas
		const hexValues = text.replace(/[\s,]+/g, ' ').trim().split(' ');

		// Convert each hex value to a number
		const opcodes = [];
		for (const hex of hexValues) {
			// Skip empty strings
			if (!hex) continue;

			// Validate hex format
			if (!/^[0-9A-Fa-f]{1,2}$/.test(hex)) {
				throw new Error(`Invalid opcode format: ${hex}`);
			}

			const value = parseInt(hex, 16);
			opcodes.push(value);
		}

		return opcodes;
	}

	// Edit memory value at address
	promptEditMemoryValue(address) {
		const currentValue = this.debugger.readMemory(address);
		const newValueStr = prompt(
			`Edit memory at $${address.toString(16).toUpperCase().padStart(4, '0')}\nCurrent value: $${currentValue.toString(16).toUpperCase().padStart(2, '0')}\nEnter new value (hex):`,
			currentValue.toString(16).toUpperCase().padStart(2, '0')
		);

		if (newValueStr === null) return; // User canceled

		try {
			const newValue = parseInt(newValueStr, 16);
			if (isNaN(newValue) || newValue < 0 || newValue > 255) {
				this.showToast('Invalid byte value (must be 00-FF)', 'danger');
				return;
			}

			this.debugger.writeMemory(address, newValue);
			this.updateMemoryView();
			this.showToast(`Memory at $${address.toString(16).toUpperCase().padStart(4, '0')} updated to $${newValue.toString(16).toUpperCase().padStart(2, '0')}`, 'success');
		} catch (e) {
			console.error('Error updating memory:', e);
			this.showToast('Error updating memory value', 'danger');
		}
	}

	// Show toast notification
	showToast(message, type = 'info') {
		// Create toast container if it doesn't exist
		let toastContainer = document.querySelector('.toast-container');
		if (!toastContainer) {
			toastContainer = document.createElement('div');
			toastContainer.className = 'toast-container position-fixed bottom-0 end-0 p-3';
			document.body.appendChild(toastContainer);
		}

		// Create toast element
		const toastId = 'toast-' + Date.now();
		const toast = document.createElement('div');
		toast.className = `toast align-items-center text-white bg-${type} border-0`;
		toast.setAttribute('id', toastId);
		toast.setAttribute('role', 'alert');
		toast.setAttribute('aria-live', 'assertive');
		toast.setAttribute('aria-atomic', 'true');

		// Toast content
		toast.innerHTML = `
			<div class="d-flex">
				<div class="toast-body">
					${message}
				</div>
				<button type="button" class="btn-close btn-close-white me-2 m-auto" data-bs-dismiss="toast" aria-label="Close"></button>
			</div>
		`;

		// Add toast to container
		toastContainer.appendChild(toast);

		// Initialize and show the toast
		const bsToast = new bootstrap.Toast(toast, {
			autohide: true,
			delay: 3000
		});
		bsToast.show();

		// Remove toast after it's hidden
		toast.addEventListener('hidden.bs.toast', () => {
			toast.remove();
		});
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

		document.getElementById('flag-n').className = flags.N ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-v').className = flags.V ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-u').className = flags.U ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-b').className = flags.B ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-d').className = flags.D ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-i').className = flags.I ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-z').className = flags.Z ? 'badge bg-success' : 'badge bg-secondary';
		document.getElementById('flag-c').className = flags.C ? 'badge bg-success' : 'badge bg-secondary';

		// Update text content to show 1/0
		document.getElementById('flag-n').textContent = flags.N ? '1' : '0';
		document.getElementById('flag-v').textContent = flags.V ? '1' : '0';
		document.getElementById('flag-u').textContent = flags.U ? '1' : '0';
		document.getElementById('flag-b').textContent = flags.B ? '1' : '0';
		document.getElementById('flag-d').textContent = flags.D ? '1' : '0';
		document.getElementById('flag-i').textContent = flags.I ? '1' : '0';
		document.getElementById('flag-z').textContent = flags.Z ? '1' : '0';
		document.getElementById('flag-c').textContent = flags.C ? '1' : '0';
	}

	updateDisassembly() {
		const view = document.getElementById('disassemblyView');
		view.innerHTML = '';

		const instructions = this.debugger.disassembleAroundPC(10, 20);
		console.log("Raw disassembly string:", this.debugger._disassembleAroundPC(10, 20));
		const pc = this.debugger.getRegisterPC();

		for (const instr of instructions) {
			const row = document.createElement('div');
			row.className = 'instruction d-flex mb-1 py-1 px-2 rounded';
			row.dataset.address = instr.address.toString(16);

			if (instr.address === pc) {
				row.className += ' current';
			}

			if (this.breakpoints.has(instr.address)) {
				row.className += ' border-warning border-start border-3';
			}

			const addrSpan = document.createElement('span');
			addrSpan.className = 'me-3 text-primary';
			addrSpan.style.width = '60px';
			addrSpan.textContent = `$${instr.address.toString(16).toUpperCase().padStart(4, '0')}`;

			const opcodeSpan = document.createElement('span');
			opcodeSpan.className = 'me-3 text-secondary';
			opcodeSpan.style.width = '30px';
			opcodeSpan.textContent = `${instr.opcode.toString(16).toUpperCase().padStart(2, '0')}`;

			const mnemonicSpan = document.createElement('span');
			mnemonicSpan.className = 'me-3 fw-bold';
			mnemonicSpan.style.width = '50px';
			mnemonicSpan.textContent = instr.mnemonic;

			const operandsSpan = document.createElement('span');
			operandsSpan.className = '';
			operandsSpan.textContent = instr.formatted.substring(instr.mnemonic.length);

			const bytesSpan = document.createElement('span');
			bytesSpan.className = 'ms-auto text-muted small';
			bytesSpan.textContent = `${instr.bytes} bytes, ${instr.cycles} cycles`;

			row.appendChild(addrSpan);
			row.appendChild(opcodeSpan);
			row.appendChild(mnemonicSpan);
			row.appendChild(operandsSpan);
			row.appendChild(bytesSpan);

			view.appendChild(row);
		}
	}

	updateMemoryView() {
		const view = document.getElementById('memoryView');
		view.innerHTML = '';

		// Add headers
		const headerRow = document.createElement('div');
		headerRow.className = 'memory-row mb-2 px-2 py-1 bg-primary bg-opacity-10 rounded';

		const addrHeader = document.createElement('div');
		addrHeader.className = 'memory-header fw-bold text-primary';
		addrHeader.style.width = '80px';
		addrHeader.textContent = 'Address';

		headerRow.appendChild(addrHeader);

		for (let i = 0; i < 16; i++) {
			const header = document.createElement('div');
			header.className = 'memory-header fw-bold text-primary';
			header.style.width = '30px';
			header.textContent = i.toString(16).toUpperCase();
			headerRow.appendChild(header);
		}

		const asciiHeader = document.createElement('div');
		asciiHeader.className = 'memory-header fw-bold text-primary ms-2';
		asciiHeader.style.width = '180px';
		asciiHeader.textContent = 'ASCII';

		headerRow.appendChild(asciiHeader);
		view.appendChild(headerRow);

		// Calculate how many rows to display
		const rowCount = Math.min(16, Math.ceil(this.memoryPageSize / 16));

		// Add memory rows
		for (let row = 0; row < rowCount; row++) {
			const rowStartAddr = this.currentMemoryPage + (row * 16);

			const memoryRow = document.createElement('div');
			memoryRow.className = 'memory-row mb-1 px-2 py-1 rounded';

			const addrCell = document.createElement('div');
			addrCell.className = 'memory-address text-primary';
			addrCell.style.width = '80px';
			addrCell.textContent = `$${rowStartAddr.toString(16).toUpperCase().padStart(4, '0')}`;

			memoryRow.appendChild(addrCell);

			let asciiText = '';

			for (let col = 0; col < 16; col++) {
				const addr = rowStartAddr + col;

				if (addr <= 0xFFFF) {
					const value = this.debugger.readMemory(addr);

					const cell = document.createElement('div');
					cell.className = 'memory-cell text-center';
					cell.style.width = '30px';
					cell.dataset.address = addr;
					cell.textContent = value.toString(16).toUpperCase().padStart(2, '0');
					cell.setAttribute('data-bs-toggle', 'tooltip');
					cell.setAttribute('data-bs-placement', 'top');
					cell.setAttribute('title', `Click to edit memory at $${addr.toString(16).toUpperCase().padStart(4, '0')}`);

					// Highlight current PC
					if (addr === this.debugger.getRegisterPC()) {
						cell.className += ' bg-danger bg-opacity-25 fw-bold';
					}

					// Highlight SP for stack memory page
					if (this.currentMemoryPage === 0x0100 && addr === 0x0100 + this.debugger.getRegisterSP()) {
						cell.className += ' bg-success bg-opacity-25 fw-bold';
					}

					memoryRow.appendChild(cell);

					// ASCII representation
					asciiText += (value >= 32 && value <= 126) ? String.fromCharCode(value) : '.';
				} else {
					const cell = document.createElement('div');
					cell.className = 'memory-cell text-center text-muted';
					cell.style.width = '30px';
					cell.textContent = '--';
					memoryRow.appendChild(cell);

					asciiText += ' ';
				}
			}

			const asciiCell = document.createElement('div');
			asciiCell.className = 'memory-ascii ms-2 font-monospace';
			asciiCell.style.width = '180px';
			asciiCell.textContent = asciiText;

			memoryRow.appendChild(asciiCell);
			view.appendChild(memoryRow);
		}

		// Re-initialize tooltips
		this.initTooltips();
	}

	updateStats() {
		const state = this.debugger.getState();
		if (!state) return;

		document.getElementById('instructionCount').textContent = state.stats.instructionCount.toString();
		document.getElementById('cycleCount').textContent = state.stats.cycleCount.toString();

		const isRunning = state.running;
		document.getElementById('runButton').disabled = isRunning;
		document.getElementById('stopButton').disabled = !isRunning;

		// Update button styles based on running state
		if (isRunning) {
			document.getElementById('runButton').classList.add('disabled');
			document.getElementById('stepButton').classList.add('disabled');
			document.getElementById('stopButton').classList.remove('disabled');
		} else {
			document.getElementById('runButton').classList.remove('disabled');
			document.getElementById('stepButton').classList.remove('disabled');
			document.getElementById('stopButton').classList.add('disabled');
		}
	}
}

window.debuggerUI = new DebuggerUI();
