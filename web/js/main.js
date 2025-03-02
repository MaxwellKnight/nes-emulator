document.addEventListener('DOMContentLoaded', function() {
	console.log('DOM content loaded, setting up initialization...');

	// Create a promise for debugger initialization
	const debuggerReady = new Promise((resolve, reject) => {
		const timeout = setTimeout(() => {
			reject(new Error('Debugger initialization timed out'));
		}, 30000);

		// Load the WASM module
		const script = document.createElement('script');
		script.src = 'js/cpu_wasm.js';
		script.onerror = () => reject(new Error('Failed to load WASM script'));
		script.onload = () => {
			// Initialize the CPU emulator
			if (window.CPUEmulator) {
				window.CPUEmulator()
					.then(module => {
						window.Module = module;
						clearTimeout(timeout);
						resolve(module);
					})
					.catch(err => reject(err));
			} else if (window.Module && typeof window.Module.ccall === 'function') {
				clearTimeout(timeout);
				resolve(window.Module);
			} else {
				reject(new Error('CPUEmulator or Module not available'));
			}
		};

		document.body.appendChild(script);
	});

	// Handle the initialization
	debuggerReady
		.then(module => {
			// Initialize the debugger
			window.nesDebugger = window.nesDebugger || new NESDebugger();
			window.nesDebugger.module = module;
			window.nesDebugger.setupFunctions();
			window.nesDebugger.isLoaded = true;

			// Initialize the UI
			window.debuggerUI = window.debuggerUI || new DebuggerUI();
			window.debuggerUI.updateUI();

			// Initialize tooltips if bootstrap is available
			if (typeof bootstrap !== 'undefined') {
				const tooltipTriggerList = document.querySelectorAll('[data-bs-toggle="tooltip"]');
				tooltipTriggerList.forEach(tooltipTriggerEl => {
					new bootstrap.Tooltip(tooltipTriggerEl);
				});
			}

			// Mark as initialized and dispatch event
			module.__initialized = true;
			window.dispatchEvent(new CustomEvent('debugger-ready'));
		})
		.catch(error => {
			console.error('Initialization error:', error);
			const statusEl = document.getElementById('status-message');
			if (statusEl) {
				statusEl.textContent = 'Initialization error: ' + error.message;
			}
		});
});
