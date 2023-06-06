/*
Authors: Advik Chitre
Date created: 03/05/23
*/

//-------------------------------- Imports ----------------------------------------------

import React, { useState } from 'react';
import { Link } from 'react-router-dom';
import ReactPolling from "react-polling/lib/ReactPolling";
import '../components/Connected/grid_Connected.css';
import '../components/grid.css';

//-------------------------------- Main -------------------------------------------------

/* 
Connected Page - Page for when user client is connected to rover client through server.
    			 Displays live data from rover and has pause/play buttons.
*/

const Connected = () => {

	//---------------------------- Get Stored Values ------------------------------------

	const ServerIP = localStorage.getItem('ServerIP')
	const MAC = localStorage.getItem('MAC');
	console.log('CONNECTED MAC = ' + MAC)
	const nickname = localStorage.getItem('nickname');
	console.log('CONNECTED nickname = ' + nickname)
	
	//---------------------------- Polling: Diagnostics ---------------------------------

	/* 
	Diagnostics - Gives to server: MAC, 
				  Gets from server: diagnostic data
	*/

	/* Create updating variables */
	const [DiagnosticData, UpdateDiagnosticData] = useState([]);

	/* Send POST request to get Diagnostics data */
	const DiagnosticURL = "http://" + ServerIP + ":5000/client/diagnostics";
	const fetchDiagnosticData = () => {
		console.log("URL = " + DiagnosticURL);
		return fetch(DiagnosticURL, {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ "MAC": MAC })
		});
	};

	/* On GOOD POST response */
	const DiagnosticPollingSuccess = (jsonResponse) => {
		console.log("JSON RESPONSE: " + JSON.stringify(jsonResponse));
		UpdateDiagnosticData(jsonResponse);
		return true;
	}

	/* On BAD POST response */
	const DiagnosticPollingFailure = () => {
		console.log("DIAGNOSTIC POLLING FAIL");
		return true;
	}

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
			body: JSON.stringify({ "MAC": MAC })
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
			ChangeState();
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
			ChangeState();
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
	const [ConnectedState, setConnectedState] = useState('Start'); // TODO: Retreieve from server (check diagnostic data contains finish=true)

	/* Switch function (called on successful response from server pause/play POST) */
	const ChangeState = () => {
		if (ConnectedState === "Start") {
			setConnectedState("Mapping");
		}
		else if (ConnectedState === "Mapping") {
			setConnectedState("Pause");
		}
		else if (ConnectedState === "Pause") {
			setConnectedState("Mapping");
		}
	};

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
														<button onClick={handlePlay()} className='box-green buttons_Connected'>
															Start Mapping
														</button>
													</div>
												  </div>
												) : (<></>)}
				{ConnectedState === 'Mapping' ? ( <button onClick={handlePause()} className='box-green LargeButton_Connected'>
													Pause
												  </button>
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
														<button onClick={handlePlay()} className='box-green buttons_Connected'>
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
							return <p>Map</p>;
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
							return <p>CPU: {DiagnosticData.CPU}, MAC: {DiagnosticData.MAC}, Battery: {DiagnosticData.battery}, Connection: {DiagnosticData.connection}, Timestamp: {DiagnosticData.timestamp}</p>;
						}}
					/>
				</div>
			</div>
		</div>
	);
};

export default Connected;