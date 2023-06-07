/*
Authors: Advik Chitre
Date created: 03/05/23
*/

//-------------------------------- Imports ----------------------------------------------

import React, { useState } from 'react';
import ReactPolling from "react-polling/lib/ReactPolling";
import RoverCarousel from '../components/Home/RoverCarousel';
import ReplayCarousel from '../components/Home/ReplayCarousel';
import '../components/Home/grid_Home.css';
import '../components/grid.css';
import AddIcon from '../components/Home/AddIcon.png';

//-------------------------------- Main -------------------------------------------------

/* 
Home Page - Landing page when user first requests page from server.
    		Polls server for all rovers and replays, allowing user to select.
*/

const Home = () => {

	//---------------------------- Get Stored Values ------------------------------------

	const GetServerIP = localStorage.getItem('ServerIP');

	//---------------------------- Polling: Rovers --------------------------------------

	/* 
	Rovers - Gets from server: MAC, Nickname, Connection status
	*/

	/* Create updating variables */
	const [rovers_list, Update_rovers_list] = useState([]);

	/* Send POST request to get Rover Data list */
	const RoverURL = "http://" + GetServerIP + ":5000/client/allrovers";
	const fetchRoverList = () => {
		console.log("URL = " + RoverURL);
		return fetch(RoverURL);
	}

	/* On GOOD POST response */
	const RoverPollingSuccess = (jsonResponse) => {
		console.log("JSON RESPONSE: " + JSON.stringify(jsonResponse));
		Update_rovers_list(jsonResponse);
		return true;
	}

	/* On BAD POST response */
	const RoverPollingFailure = () => {
		console.log("ROVER POLLING FAIL");
		return true;
	}

	//---------------------------- Polling: Replays -------------------------------------

	/* 
	Replays - Gets from server: MAC, Nickname, Connection status
	*/

	/* Create updating variables */
	const [replays_list, Update_replays_list] = useState([]);

	/* Send POST request to get Replay Data list */
	const ReplayURL = "http://" + GetServerIP + ":5000/client/sessions";
	const fetchReplayList = () => {
		console.log("URL = " + ReplayURL);
		return fetch(ReplayURL);
	}

	/* On GOOD POST response */
	const ReplayPollingSuccess = (jsonResponse) => {
		console.log("JSON RESPONSE: " + JSON.stringify(jsonResponse));
		Update_replays_list(jsonResponse);
		return true;
	}

	/* On BAD POST response */
	const ReplayPollingFailure = () => {
		console.log("REPLAY POLLING FAIL");
		return true;
	}

	//---------------------------- Button Functions: onClick ----------------------------

	/* Add Rover button */
	const handleAddRover = () => {
		console.log("Add New Rover");
	}

	//---------------------------- Display ----------------------------------------------

	return (
		<div className="background">
			<div className="wrapper">
				{/* Display Text */}
				<div className="box Group1_Home">
					Group 1				
				</div>
				<div className="box RoversText_Home">
					Rovers
				</div>
				{/* Display rovers List */}
				<div className="box RoverCarousel_Home">
					<ReactPolling
						url={RoverURL}
						interval= {3000} // in milliseconds(ms)
						retryCount={3} // this is optional
						onSuccess = {RoverPollingSuccess}
						onFailure= {RoverPollingFailure}
						promise={fetchRoverList} // custom api calling function that should return a promise
						render={({ startPolling, stopPolling, isPolling }) => {
							return <RoverCarousel rovers={rovers_list}/>;
						}}
					/>
				</div>
				{/* Add new rover button */}
				<div className="box-nopadding AddButton_Home">
					<button onClick={handleAddRover} className='buttons_Home'>
						<img
							src={AddIcon}
							alt={"New Rover"}
							style={{
								width: '100px', // Set the desired width
								height: '100px', // Set the desired height
								objectFit: 'cover', // Adjust how the image fits within the container
							}}
						/>
						<div className="overlay-Add_Home">
							<p>New Rover</p>
						</div>
					</button>
				</div>
				{/* Display text */}
				<div className="box ReplaysText_Home">
					Replays
				</div>
				{/* Display replays List */}
				<div className="box ReplayCarousel_Home">
					<ReactPolling
						url={ReplayURL}
						interval= {3000} // in milliseconds(ms)
						retryCount={3} // this is optional
						onSuccess = {ReplayPollingSuccess}
						onFailure= {ReplayPollingFailure}
						promise={fetchReplayList} // custom api calling function that should return a promise
						render={({ startPolling, stopPolling, isPolling }) => {
							return <ReplayCarousel replays={replays_list} />;
						}}
					/>
				</div>
			</div>
		</div>
	);
};

export default Home;