import React from 'react';
import { useState } from "react";
import { Link } from 'react-router-dom';
import '../components/Connected/grid_Connected.css';
import '../components/grid.css';
import Replay from './Replay';

const Connected = () => {
	const MAC = localStorage.getItem('MAC');
	const nickname = localStorage.getItem('nickname');

	console.log('CONNECTED MAC = ' + MAC)
	console.log('CONNECTED nickname = ' + nickname)
	
	const handlePause = () => {
		console.log('Pause');
		// TODO: Send Pause Post to server
	};

	const handleStart = () => {
		console.log('Start');
		// TODO: Send Pause Post to server
	};

	const handlePlay = () => {
		console.log('Play');
		// TODO: Send Pause Post to server
	};

	const handleViewReplay = () => {
		console.log("View Replay")
		// TODO: request replay of Session
	}

	// Get starting state
	const [ConnectedState, setConnectedState] = useState('Start'); // TODO: Retreieve from server

	const ChangeState = () => {
		if (ConnectedState == "Start") {
			setConnectedState("Mapping");
			handleStart();
		}
		else if (ConnectedState == "Mapping") {
			setConnectedState("Pause");
			handlePause();
		}
		else if (ConnectedState == "Pause") {
			setConnectedState("Mapping");
			handlePlay();
		}
	};

	return (
		<div className="background">
			<div className="wrapper">
				<div className="box-nobackground DisplayMAC_Connected">
					{nickname} / {MAC}
				</div>
				{/* State-dependant Buttons */}
				{ConnectedState === 'Start'   ? ( <div className="wrapper">
													<div className='box-red SmallRightButton_Connected'>
														<Link to='/' className="page-link" draggable={false}>
															<button onClick={ChangeState} className='box-red buttons_Connected'>
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
				{ConnectedState === 'Pause'   ? (<div className="wrapper">
													<div className='box-red SmallRightButton_Connected'>
														<Link to='/' className="page-link" draggable={false}>
															<button onClick={ChangeState} className='box-red buttons_Connected'>
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
				{ConnectedState === 'Finish'  ? (<div className="wrapper">
													<div className='box-red SmallRightButton_Connected'>
														<Link to='/' className="page-link" draggable={false}>
															<button onClick={ChangeState} className='box-red buttons_Connected'>
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
					Map	
				</div>
				<div className="box Data_Connected">
					<p>Data</p><p>Data</p><p>Data&#10;Data&#10;</p>
				</div>
			</div>
		</div>
	);
};

export default Connected;
