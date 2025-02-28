document.addEventListener('DOMContentLoaded', function() {
	// Extended initialization timeout
	const initializationTimeout = setTimeout(() => {
		console.error('Debugger initialization timed out');
		// Optional: Provide user feedback or fallback
		alert('Failed to initialize NES Debugger. Please refresh the page.');
	}, 10000); // 10 seconds timeout

	function safeInitialize() {
		try {
			// Detailed module status check
			console.log('Module initialization attempt', {
				moduleExists: !!window.Module,
				calledRun: window.Module?.calledRun,
				initialized: window.Module?.__initialized
			});

			// Defensive checks
			if (!window.Module || window.Module.__initialized) {
				console.warn('Module not ready or already initialized');
				return;
			}

			// Initialize components
			window.nesDebugger = new NESDebugger();
			window.debuggerUI = new DebuggerUI();

			// Initialize tooltips
			const tooltipTriggerList = document.querySelectorAll('[data-bs-toggle="tooltip"]');
			tooltipTriggerList.forEach(tooltipTriggerEl => {
				new bootstrap.Tooltip(tooltipTriggerEl);
			});

			console.log('Debugger initialized successfully');

			// Clear timeout on successful initialization
			clearTimeout(initializationTimeout);
		} catch (error) {
			console.error('Debugger initialization failed:', error);
			console.error('Error details:', error.stack);
		}
	}

	// Multiple initialization strategies
	window.addEventListener('wasm-ready', safeInitialize);

	// Fallback initialization
	if (window.Module?.calledRun) {
		safeInitialize();
	}
});
