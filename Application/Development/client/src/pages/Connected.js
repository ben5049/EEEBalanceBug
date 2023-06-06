import React, { useState } from 'react';
import { Link } from 'react-router-dom';
import '../components/Connected/grid_Connected.css';
import '../components/grid.css';
import ReactPolling from "react-polling/lib/ReactPolling";

const Connected = () => {
	const ServerIP = localStorage.getItem('ServerIP');
	// Retrieve selected rover
	const MAC = localStorage.getItem('MAC');
	const nickname = localStorage.getItem('nickname');
	console.log('CONNECTED MAC = ' + MAC)
	console.log('CONNECTED nickname = ' + nickname)
	
	// Polling - Gets from server: diagnostic data
	const [DiagnosticData, UpdateDiagnosticData] = useState([]);
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
	const DiagnosticPollingSuccess = (jsonResponse) => {
		console.log("JSON RESPONSE: " + JSON.stringify(jsonResponse));
		UpdateDiagnosticData(jsonResponse);
		return true;
	}
	const DiagnosticPollingFailure = () => {
		console.log("DIAGNOSTIC POLLING FAIL");
		return true;
	}

	// Polling - Gets from server: map data
	const [MapData, UpdateMappingData] = useState([]);
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
	const MappingPollingSuccess = (jsonResponse) => {
		console.log("JSON RESPONSE: " + JSON.stringify(jsonResponse));
		UpdateMappingData(jsonResponse);
		return true;
	}
	const MappingPollingFailure = () => {
		console.log("MAPPING POLLING FAIL");
		return true;
	}

	// Button click handles
	const handlePause = () => {
		console.log('Pause');
		// Send Pause Post to server
		const url = "http://" + ServerIP + ":5000/client/pause"; // Pause endpoint
		console.log("URL = " + url);
		fetch(url, {
		  method: 'POST',
		  headers: {
			'Content-Type': 'application/json'
		  },
		  body: JSON.stringify({ "MAC": MAC })
		})
		.then(response => response.json())  // Parse response as JSON
		.then(data => {
		  console.log("Response from server:", data);  // Print the response data
		  // Handle the response data as needed
		})
		.catch(error => {
		  console.log("Error:", error);
		  // Handle any errors
		});
	  };


	const handlePlay = () => {
		console.log('Play');
		// Send Play Post to server
		const url = "http://" + ServerIP + ":5000/client/play"; // Play endpoint
		console.log("URL = " + url);
		fetch(url, {
			method: 'POST',
			headers: {
			'Content-Type': 'application/json'
			},
			body: JSON.stringify({ "MAC": MAC })
		})
		.then(response => response.json())  // Parse response as JSON
		.then(data => {
			console.log("Response from server:", data);  // Print the response data
			// Handle the response data as needed
		})
		.catch(error => {
			console.log("Error:", error);
			// Handle any errors
		});
	};

	const handleViewReplay = () => {
		console.log("View Replay")
		// TODO: set ReplayID of Session (given by server)
		const ReplayID = "01";
		localStorage.setItem('ReplayID', ReplayID);
	}
	const handleMenu = () => {
		console.log("Menu")
	}

	// Get starting state
	const [ConnectedState, setConnectedState] = useState('Start'); // TODO: Retreieve from server

	// Switch state on button click
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

	return (
		<div className="background">
			<div className="wrapper_Connected">
				<div className="box-nobackground DisplayMAC_Connected">
					{nickname} / {MAC}
				</div>
				{/* State-dependant Buttons */}
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
				{ConnectedState === 'Mapping' ? ( <button onClick={ChangeState} className='box-green LargeButton_Connected'>
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
