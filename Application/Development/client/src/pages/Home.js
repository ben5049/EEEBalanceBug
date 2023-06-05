import React, { useState } from 'react';
import RoverCarousel from '../components/Home/RoverCarousel';
import ReplayCarousel from '../components/Home/ReplayCarousel';
import '../components/Home/grid_Home.css';
import '../components/grid.css';
import AddIcon from '../components/Home/AddIcon.png';
import ReactPolling from "react-polling/lib/ReactPolling";

const Home = () => {
	const GetServerIP = localStorage.getItem('ServerIP');

	// Gets from server: MAC, Nickname, Connection status
	const [rovers_list, Update_rovers_list] = useState([]);
	const RoverURL = "http://" + GetServerIP + ":5000/client/allrovers";
	const fetchRoverList = () => {
		console.log("URL = " + RoverURL);
		return fetch(RoverURL);
	}
	const RoverPollingSuccess = (jsonResponse) => {
		console.log("JSON RESPONSE: " + JSON.stringify(jsonResponse));
		Update_rovers_list(jsonResponse);
		return true;
	}
	const RoverPollingFailure = () => {
		console.log("ROVER POLLING FAIL");
		return true;
	}
	
	{/*
	const rovers_list = [
		{MAC: '11:11:11:11:11:11', nickname: 'David2', overlayText: 'CONNECT' },
		{MAC: '13:13:13:13:13:13', nickname: 'Kyle12', overlayText: 'OFFLINE' },
		{MAC: '00:00:00:00:00:00', nickname: 'David43', overlayText: 'OFFLINE' },
		{MAC: '11:11:11:11:11:11', nickname: 'David3', overlayText: 'CONNECT' },
		{MAC: '13:13:13:13:13:13', nickname: 'Kyle65', overlayText: 'OFFLINE' },
		{MAC: '00:00:00:00:00:00', nickname: 'David15', overlayText: 'OFFLINE' },
		{MAC: '11:11:11:11:11:11', nickname: 'David4', overlayText: 'CONNECT' },
		{MAC: '13:13:13:13:13:13', nickname: 'Kyle2', overlayText: 'OFFLINE' },
		{MAC: '00:00:00:00:00:00', nickname: 'David15', overlayText: 'OFFLINE' },
	];
	*/}

	const replays_list = [
		{ID: '1', name: 'run1'},
		{ID: '2', name: 'trial'},
		{ID: '3', name: 'test'},
		{ID: '4', name: 'It works!!'},
		{ID: '5', name: "didn't work :("},
		{ID: '6', name: 'aaaaa'},
		{ID: '7', name: 'ababa'},
		{ID: '8', name: 'what'},
		{ID: '9', name: 'wtf'},
	];

	const handleAddRover = () => {
		console.log("Add New Rover");
	}
	

	return (
		<div className="background">
			<div className="wrapper">
				<div className="box Group1_Home">
					Group 1				
				</div>
				<div className="box RoversText_Home">
					Rovers
				</div>
				<div className="box RoverCarousel_Home">
					<RoverCarousel rovers={rovers_list}/>
				</div>
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
				<div className="box ReplaysText_Home">
					Replays
				</div>
				<div className="box ReplayCarousel_Home">
					<ReactPolling
						url={RoverURL}
						interval= {3000} // in milliseconds(ms)
						retryCount={3} // this is optional
						onSuccess = {RoverPollingSuccess}
						onFailure= {RoverPollingFailure}
						promise={fetchRoverList} // custom api calling function that should return a promise
						render={({ startPolling, stopPolling, isPolling }) => {
							return <ReplayCarousel replays={replays_list} />;
						}}
					/>
					{/*<ReplayCarousel replays={replays_list} />*/}
				</div>
			</div>
		</div>
	);
};

export default Home;
