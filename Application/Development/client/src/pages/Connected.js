/*
Authors: Advik Chitre
Date created: 03/05/23
*/

//-------------------------------- Imports ----------------------------------------------

import React, { useState, useEffect, useRef } from 'react';
import { Link } from 'react-router-dom';
import ReactPolling from "react-polling/lib/ReactPolling";
import '../components/Connected/grid_Connected.css';
import '../components/grid.css';
import RoverImg from '../components/Connected/location.png';

//-------------------------------- Main -------------------------------------------------

/* 
Connected Page - Page for when user client is connected to rover client through server.
    			 Displays live data from rover and has pause/play buttons.
*/

const Connected = () => {

	//---------------------------- Get Stored Values ------------------------------------

	const ServerIP = localStorage.getItem('ServerIP')
	const MAC = localStorage.getItem('MAC');
	console.log('CONNECTED MAC = ' + MAC);
	const nickname = localStorage.getItem('nickname');
	console.log('CONNECTED nickname = ' + nickname);
	const SessionID = localStorage.getItem('SessionID');
	console.log('CONNECTED SessionID = ' + SessionID);
	
	//---------------------------- Polling: Diagnostics ---------------------------------

	/* 
	Diagnostics - Gives to server: MAC, 
				  Gets from server: diagnostic data
	*/

	/* Create updating variables */
	const [DiagnosticData, UpdateDiagnosticData] = useState([{"MAC":1234567,"nickname":"MiWhip","connected":true, "sessionid":12, "CPU":10}]);

	/* Send POST request to get Diagnostics data */
	const DiagnosticURL = "http://" + ServerIP + ":5000/client/diagnostics";
	const fetchDiagnosticData = () => {
		console.log("URL = " + DiagnosticURL + " used: " + SessionID);
		return fetch(DiagnosticURL, {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ "sessionid": SessionID })
		});
	};

	/* On GOOD POST response */
	const DiagnosticPollingSuccess = (jsonResponse) => {
		console.log("JSON RESPONSE: " + JSON.stringify(jsonResponse));
		UpdateDiagnosticData(jsonResponse); /* Displays most recent value */
		if (jsonResponse.response.isfinished) {// TODO: Change isfinished once added to response on server side
			setConnectedState("Finish");
		}
		return true;
	}

	/* On BAD POST response */
	const DiagnosticPollingFailure = () => {
		console.log("DIAGNOSTIC POLLING FAIL");
		return true;
	}

	/* Updating Diagnostic variables */
	const [Timestamp, updateTimestamp] = useState("N/A");/* Number of Replay values (in MapData) */
	const [Battery, updateBattery] = useState("N/A");/* Number of Replay values (in MapData) */
	const [Connection, updateConnection] = useState("N/A");/* Number of Replay values (in MapData) */
	useEffect(() => {
		if (DiagnosticData === "[]"){
			updateTimestamp("N/A");
			updateBattery("N/A");
			updateConnection("N/A");
		}
		else{
			let Diagnostic_recent = DiagnosticData[0];
			console.log("DIAGNOSTICS HERE !!! ")
			console.log(Diagnostic_recent);
			updateTimestamp(Diagnostic_recent.timestamp);
			updateBattery(Diagnostic_recent.battery);
			updateConnection(Diagnostic_recent.connected);
		}
	}, [DiagnosticData]);

	//---------------------------- Polling: Mapping data --------------------------------

	/* 
	Diagnostics - Gives to server: MAC, 
				  Gets from server: map data
	*/

	/* Create updating variables */
	const [MapData, UpdateMappingData] = useState([]);

	/* Send POST request to get Map data */
	const MappingURL = "http://" + ServerIP + ":5000/client/replay";
	const fetchMappingData = () => {
		console.log("URL = " + MappingURL);
		return fetch(MappingURL, {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ "sessionid": SessionID })
		});
	};

	/* On GOOD POST response */
	const MappingPollingSuccess = (jsonResponse) => {
		console.log("JSON RESPONSE: " + JSON.stringify(jsonResponse));
		UpdateMappingData(jsonResponse);
		return true;
	}

	/* On BAD POST response */
	const MappingPollingFailure = () => {
		console.log("MAPPING POLLING FAIL");
		return true;
	}

	/* Get max magnitude in MapData when changed */
	const [MapDataMax, setMapDataMax] = useState(1);/* Number of Replay values (in MapData) */
	useEffect(() => {
		Object.entries(MapData).forEach(([key, value]) => {
			const firstTwoElements = value.slice(0, 2);
			const CoordinateMax = Math.max(...firstTwoElements);
			if (CoordinateMax > MapDataMax) {
				setMapDataMax(CoordinateMax);
				console.log("MapDataMax = " + MapDataMax);
			}
		})
	}, [MapData]);

	//---------------------------- Button Functions: onClick ----------------------------

	/* Pause Button */
	const handlePause = () => {
		console.log('Pause');
		/* Send Pause POST to server */
		const url = "http://" + ServerIP + ":5000/client/pause"; /* Pause endpoint */
		console.log("URL = " + url);
		fetch(url, {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ "MAC": MAC }) /* Gives MAC to server */
		})
		.then(response => response.json())  /* Parse response as JSON */
		.then(data => {
			console.log("Response from server:", data);  /* Log response data */
		})
		.catch(error => {
			console.log("Error:", error);
			/* Error handling */
		});
	};

	/* Pause Button */
	const handleEmergencyStop = () => {
		console.log('Emergency Stop');
		/* Send Pause POST to server */
		const url = "http://" + ServerIP + ":5000/client/estop"; /* Emergency Stop endpoint */
		console.log("URL = " + url);
		fetch(url, {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ "MAC": MAC }) /* Gives MAC to server */
		})
		.then(response => response.json())  /* Parse response as JSON */
		.then(data => {
			console.log("Response from server:", data);  /* Log response data */
		})
		.catch(error => {
			console.log("Error:", error);
			/* Error handling */
		});
	};
	
	/* Play Button */
	const handlePlay = () => {
		console.log('Play');
		/* Send Play POST to server */
		const url = "http://" + ServerIP + ":5000/client/play"; /* Play endpoint */
		console.log("URL = " + url);
		fetch(url, {
			method: 'POST',
			headers: {
			'Content-Type': 'application/json'
			},
			body: JSON.stringify({ "MAC": MAC }) /* Gives MAC to server */
		})
		.then(response => response.json()) /* Parse response as JSON */
		.then(data => {
			console.log("Response from server:", data); /* Log response data */
		})
		.catch(error => {
			console.log("Error:", error);
			/* Error handling */
		});
	};

	/* View replay button */
	const handleViewReplay = () => {
		console.log("View Replay")
		// TODO: set ReplayID of Session (given by server)
		const ReplayID = "01";
		localStorage.setItem('ReplayID', ReplayID);
	}

	/* Menu button (debug) */
	const handleMenu = () => {
		console.log("Menu")
	}

	//---------------------------- State Machine: Mapping -------------------------------

	/*
	Keeps track of whether in states: ['Start', 'Mapping', 'Pause', 'Finish']
	Another 'End' state is added to change the icon for a better UI.
	*/

	/* Create updating variables */
	const [ConnectedState, setConnectedState] = useState('Mapping'); /* Default State */ // TODO: Retreieve from server (check diagnostic data contains finish=true)

	/* Switch function (called on successful response from server pause/play POST) */
	const ChangeState = () => {
		if (ConnectedState === "Start") {
			setConnectedState("Mapping");
			handlePlay();
		}
		else if (ConnectedState === "Mapping") {
			setConnectedState("Pause");
			handlePause();
		}
		else if (ConnectedState === "Pause") {
			setConnectedState("Mapping");
			handlePlay();
		}
	};

	//---------------------------- Canvas -----------------------------------------------

	/* Create a ref for the canvas element */
	const canvasRef = useRef(null);

	/* Top level function to draw on the canvas */
	const drawOnCanvas = () => {
		const canvas = canvasRef.current;
		const ctx = canvas.getContext('2d');
	  
		/* Clear the canvas */
		ctx.clearRect(0, 0, canvas.width, canvas.height);
	  
		/* Draws black background rectangle */
		ctx.fillStyle = 'black';
		ctx.fillRect(0, 0, canvas.width, canvas.height);

		/* Re-renders map */
		renderBoundaries();
	};

	/* Draws lane boundaries given data from rover */
	const renderBoundaries = () => {
		const entries = Object.entries(MapData);
		let pos_x;
		let pos_y
		for (let i = 0; i < entries.length; i++) {
			const [, entry] = entries[i];
			pos_x = entry[0];
			pos_y = entry[1];
			const orientation = entry[3];
			const TOF_left = entry[4];
			const TOF_right = entry[5];
			draw(pos_x, pos_y, orientation, TOF_left, TOF_right);
			console.log(entry[0] + " " + entry[1] + " " + entry[3] + " " + entry[4] + " " + entry[5]);
		}
		/* Draws rover's last known position */
		renderRover(pos_x,pos_y);
	};

	/* Draws rover at last known position */
	function renderRover(x, y) {
		const canvas = canvasRef.current;
		const context = canvas.getContext('2d');

		// Add image to the canvas
		const image = new Image();
		image.src = RoverImg;
		image.onload = () => {
			context.drawImage(image, ScaleToCanvas(x), ScaleToCanvas(y), 21, 32);
    	};
	}

	/* Redraw canvas when new data from server */
	useEffect(() => {
		drawOnCanvas();
	}, [MapData]);

	//---------------------------- Canvas Drawing ---------------------------------------

	/* Draws Visual line on canvas */
	function drawLine(ctx, begin, end, stroke = 'white', width = 1) {
		if (stroke) {
			ctx.strokeStyle = stroke;
		}
		if (width) {
			ctx.lineWidth = width;
		}
		ctx.beginPath();
		ctx.moveTo(...begin);
		ctx.lineTo(...end);
		ctx.stroke();
	}

	/* draws small line in correct direction with correct distances using TOF sensors */
	function draw(position_x, position_y, orientation, tofleft, tofright) {
		const canvas = document.querySelector('#canvas');
		if (canvas.getContext) {
			const ctx = canvas.getContext('2d');
			let l = 5; /* Change to alter length of each line */
			let theta = orientation*Math.PI/180
			/* Scale */
			position_x  = ScaleToCanvas(position_x)
			position_y  = ScaleToCanvas(position_y)
			/* Draw */
			drawLine(ctx, [position_x + tofleft * Math.cos(theta), position_y + tofleft * Math.sin(theta)], [position_x + tofleft * Math.cos(theta) + l * Math.sin(theta), position_y + tofleft * Math.sin(theta) - l * Math.cos(theta)]);
			drawLine(ctx, [position_x - tofright * Math.cos(theta), position_y - tofright * Math.sin(theta)], [position_x - tofright * Math.cos(theta) + l * Math.sin(theta), position_y - tofright * Math.sin(theta) - l * Math.cos(theta)]);
		}
	}

	/* Scale to canvas as coordinate system for rover different to canvas */
	const ScaleToCanvas = (coordinate) => {
		return ( coordinate / MapDataMax ) * 700 + 30 /* add offset so not touching border */
	}

	//---------------------------- Display ----------------------------------------------

	return (
		<div className="background">
			<div className="wrapper_Connected">
				{/* Display text */}
				<div className="box-nobackground DisplayMAC_Connected">
					{nickname} / {MAC}
				</div>
				{/* State-dependant Buttons (start & menu/pause/play & menu/replay & menu) */}
				{ConnectedState === 'Start'   ? ( <div className="wrapper_Connected">
													<div className='box-red SmallRightButton_Connected'>
														<Link to='/' className="page-link" draggable={false}>
															<button onClick={handleMenu} className='box-red buttons_Connected'>
																Menu
															</button>
														</Link>
													</div>
												  
													<div className='box-green SmallLeftButton_Connected'>
														<button onClick={ChangeState} className='box-green buttons_Connected'>
															Start Mapping
														</button>
													</div>
												  </div>
												) : (<></>)}
				{ConnectedState === 'Mapping' ? ( <div className="wrapper_Connected">
												  	<button onClick={ChangeState} className='box-green PauseButton_Connected'>
														Pause
													</button>
													<button onClick={handleEmergencyStop} className='box-red EmergencyStopButton_Connected'>
														Emergency Stop
													</button>
												  </div>
										  		) : (<></>)}
				{ConnectedState === 'Pause'   ? (<div className="wrapper_Connected">
													<div className='box-red SmallRightButton_Connected'>
														<Link to='/' className="page-link" draggable={false}>
															<button onClick={handleMenu} className='box-red buttons_Connected'>
																Menu
															</button>
														</Link>
													</div>
												  
													<div className='box-green SmallLeftButton_Connected'>
														<button onClick={ChangeState} className='box-green buttons_Connected'>
															Resume
														</button>
													</div>
												  </div>
												) : (<></>)}
				{ConnectedState === 'Finish'  ? (<div className="wrapper_Connected">
													<div className='box-red SmallRightButton_Connected'>
														<Link to='/' className="page-link" draggable={false}>
															<button onClick={handleMenu} className='box-red buttons_Connected'>
																Menu
															</button>
														</Link>
													</div>
												  
													<div className='box-green SmallLeftButton_Connected'>
														<Link to='/replay' className="page-link" draggable={false}>
															<button onClick={handleViewReplay} className='box-red buttons_Connected'>
																View Replay
															</button>
														</Link>
													</div>
												  </div>
												) : (<></>)}
				{/* Display map */}
				<div className="box Map_Connected">
					<ReactPolling
						url={MappingURL}
						interval= {2000} // in milliseconds(ms)
						retryCount={3} // this is optional
						onSuccess = {MappingPollingSuccess}
						onFailure= {MappingPollingFailure}
						promise={fetchMappingData} // custom api calling function that should return a promise
						render={({ startPolling, stopPolling, isPolling }) => {
							return <canvas ref={canvasRef} id='canvas' width={715} height={715} />;
						}}
					/>
				</div>
				{/* Display data */}
				<div className="box Data_Connected">
					<ReactPolling
						url={DiagnosticURL}
						interval= {2000} // in milliseconds(ms)
						retryCount={3} // this is optional
						onSuccess = {DiagnosticPollingSuccess}
						onFailure= {DiagnosticPollingFailure}
						promise={fetchDiagnosticData} // custom api calling function that should return a promise
						render={({ startPolling, stopPolling, isPolling }) => {
							// TODO: select which diagnostics to display
							return <p>MAC: {MAC}, Battery: {Battery}, Connection: {Connection}, Timestamp: {Timestamp}</p>;
						}}
					/>
				</div>
			</div>
		</div>
	);
};

export default Connected;