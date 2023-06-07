/*
Authors: Advik Chitre
Date created: 03/05/23
*/

//-------------------------------- Imports ----------------------------------------------

import React, { useState, useEffect, useRef } from 'react';
import { Link } from 'react-router-dom';
import '../components/Replay/grid_Replay.css';
import '../components/grid.css';
import RewindIcon from '../components/Replay/RewindIcon.png';
import PauseIcon from '../components/Replay/PauseIcon.png';
import PlayIcon from '../components/Replay/PlayIcon.png';
import ReplayIcon from '../components/Replay/ReplayIcon.png';
import FastforwardIcon from '../components/Replay/FastforwardIcon.png';
import RepeatIcon from '../components/Replay/RepeatIcon.png';
import RepeatEnabledIcon from '../components/Replay/RepeatEnabledIcon.png';

//-------------------------------- Main -------------------------------------------------

/* 
Replay Page - given a replayID, this page displays a replay while also giving the user control.
    		  It also allows the user to change the shortest path start and end points.
*/

const Replay = () => {

	//---------------------------- Get Stored Values ------------------------------------

	const ServerIP = localStorage.getItem('ServerIP');
	const ID = localStorage.getItem('ReplayID');
	console.log('Replay ID = ' + ID);
	const MAC = localStorage.getItem('MAC');
	console.log('CONNECTED MAC = ' + MAC);
	const nickname = localStorage.getItem('nickname');
	console.log('CONNECTED nickname = ' + nickname);

	//---------------------------- Get Mapping Data -------------------------------------

	/* 
	Mapping - Gets from server: map data 
	*/

	/* Create updating variables */
	const [MapData, UpdateMappingData] = useState([]);

	/* Send POST request to get Map Data */
	const MappingURL = "http://" + ServerIP + ":5000/client/replay";
	useEffect(() => {
		fetch(MappingURL, {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ "MAC": MAC })
		})
		.then(response => response.json()) /* Parse the response as JSON */
		.then(data => UpdateMappingData(data)) /* Update the state with the data */
		.catch(error => console.log(error)); /* Error Handling */
	}, []);

	//---------------------------- Shortest Path ----------------------------------------

	/* Create updating variables */
	const [ShortestPath, UpdateShortestPath] = useState([]);

	/* sends request to server for shortest path to every node from a given point (arguments: Start coordinates) */
	const ShortestPathURL = "http://" + ServerIP + ":5000/client/shortestpath";
	console.log("URL = " + ShortestPathURL);
	const postStartEnd = async (StartX, StartY) => {
		try {
			/* Sends POST request to server */
			const response = await fetch(ShortestPathURL, {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ "MAC": MAC, "start_x": StartX, "start_y": StartY})
			});
			/* GOOD response from server: update shortest path */
			if (response.ok) {
				const responseData = await response.json();
				console.log("Response data:", responseData);
				UpdateShortestPath(responseData);
			} 
			/* BAD response from server: throw error */
			else {
				console.log("ShortestPath request failed");
			}
		/* Error Handling */
		} catch (error) {
			console.log("Error:", error);
		}
	};

	/* Draws a circle at coordinates */
	const renderPoints = (ctx, x, y, color) => {
		ctx.strokeStyle = color;
		ctx.lineWidth = 2;
	  
		/* Draw a circle */
		ctx.beginPath();
		ctx.arc(x, y, 5, 0, 2 * Math.PI); /* Radius = 5 */
		ctx.stroke();
	};

	//---------------------------- Button Functions: onClick ----------------------------
	
	/* Menu button */
	const handleMenu = () => {
		console.log('Menu');
	};

	/* Calculate shortest path */
	const handleShortestPath = () => {
		console.log('Shortest Path');
		/* Input Validation: Check if start and end coordinates are digits */
		if (!isDigit(startCoordinates_X) || !isDigit(startCoordinates_Y) || !isDigit(endCoordinates_X) || !isDigit(endCoordinates_Y)) {
			console.log('Invalid coordinates.');
			alert('Invalid coordinates. Please enter digits.');
			return;
		}
		postStartEnd(startCoordinates_X, startCoordinates_Y);
		// TODO: Visualise shortest path
	};

	/* Pause button */
	const handlePause = () => {
		console.log('Pause');
	};

	/* Play button */
	const handlePlay = () => {
		console.log('Play');
	};

	/* Replay button */
	const handleReplay = () => {
		console.log('Replay');
		setSliderValue(parseInt(0));
	};

	//---------------------------- Button Functions: onMouseDown & onMouseUp ------------
	
	/* Initialise variables */
	const FastforwardStrength = 4; /* Slider increment changed each time Fastforward and Rewind buttons are pressed */
	const FastforwardInterval = 100; /* ms interval between slider changes */

	/* Rewind */
	const rewindIntervalRef = useRef(null); /* Use useRef to keep track of the interval ID for the button actions */
	const handleRewindStart = () => {
		console.log('Rewind Start');
		if (!rewindIntervalRef.current) { /* Check if an interval is already running to avoid starting multiple intervals */
		  rewindIntervalRef.current = setInterval(() => {
			/* Update the slider value based on your rewind logic
			For example, decrease the slider value by a certain amount */
			setSliderValue((prevValue) => {
			  const newValue = prevValue - FastforwardStrength; /* Get new value of slider  */
			  return newValue < 0 ? 0 : newValue; /* Ensure the new value stays above 0 */
			});
		  }, FastforwardInterval); /* Interval duration */
		}
	};
	const handleRewindStop = () => {
		console.log('Rewind Stop');
		/* Clear the interval when the button is released */
		clearInterval(rewindIntervalRef.current);
		rewindIntervalRef.current = null;
	  };

	  /* Fastforward */
	  const fastforwardIntervalRef = useRef(null); /* Use useRef to keep track of the interval ID for the button actions */
	  const handleFastforwardStart = () => {
		console.log('Fast Forward Start');
		if (!fastforwardIntervalRef.current) { /* Check if an interval is already running to avoid starting multiple intervals */
		  fastforwardIntervalRef.current = setInterval(() => {
			/* Update the slider value based on your fast forward logic
			For example, increase the slider value by a certain amount */
			setSliderValue((prevValue) => {
			  const newValue = prevValue + 4; /* Get new value of slider  */
			  return newValue >= SliderValueMax ? SliderValueMax : newValue; /* Ensure the new value stays below SliderValueMax */
			});
		  }, FastforwardInterval); /* Interval duration */
		}
	  };
	  const handleFastforwardStop = () => {
		console.log('Fast Forward Stop');
		/* Clear the interval when the button is released */
		clearInterval(fastforwardIntervalRef.current);
		fastforwardIntervalRef.current = null;
	  };

	//---------------------------- State Machine: Playback ------------------------------

	/*
	Keeps track of whether in states: ['Play', 'Pause']
	Another 'End' state is added to change the icon for a better UI.
	*/

	/* Set starting state */
	const [PlayState, setPlayState] = useState('Pause');

	/* Switch state on button click */
	const ChangePlayState = () => {
		if (PlayState === "Pause") {
			setPlayState("Play");
			handlePlay();
		}
		else if (PlayState === "Play") {
			setPlayState("Pause");
			handlePause();
		}
		else if (PlayState === "End") {
			setPlayState("Play");
			handleReplay();
		}
	};

	//---------------------------- State Machine: Toggle Repeat -------------------------

	/*
	Keeps track of whether in Repeat Toggle states: [true, false]
	*/

	/* Set starting state */
	const [RepeatToggleState, setRepeatToggleState] = useState(false);

	/* Switch state on button click */
	const ChangeRepeatToggleState = () => {
		if (RepeatToggleState === true) {
			setRepeatToggleState(false);
		}
		else if (RepeatToggleState === false) {
			setRepeatToggleState(true);
		}
	};

	//---------------------------- Slider -----------------------------------------------

	/* Initialise variables */
	const [SliderValue, setSliderValue] = useState(0); /* Position in replay (initial = start) */
	const SliderValueMax = 700; /* Number of Replay values, TODO: get from server */
	const SliderRate = 40; /* Time Interval (ms) */

	/* Slider onChange (Dragged by user) */
	const handleSliderChange = (event) => {
		setSliderValue(parseInt(event.target.value));
		if (RepeatToggleState === false) {
			/* Switches to replay button if at end */
			if (event.target.value === SliderValueMax) {
				setPlayState("End");
			}
			else {
				setPlayState("Pause");
			}
		}
		else {
			/* Pauses playback if user changes slider */
			setPlayState("Pause");
		}
	};

	/* Increment SliderValue with SliderRate delay between increments (when PlayState is "Play") */
	useEffect(() => {
		let interval = null;
		if (PlayState === "Play") {
			interval = setInterval(() => {
				setSliderValue((prevValue) => {
				const newValue = prevValue + 1; /* Increment */
				if (RepeatToggleState === false) {
					if (newValue === SliderValueMax) {
						setPlayState("End");
					}
					return newValue > SliderValueMax ? SliderValueMax : newValue; /* Ensure increment stops at SliderValueMax */
				}
				else if (RepeatToggleState === true) {
					return newValue > SliderValueMax ? 0 : newValue; /* Restarts if Toggle Repeat on */
				}
				});
			}, SliderRate); /* Delay between increments */
		}
		return () => {
			clearInterval(interval);
		};
	}, [PlayState, RepeatToggleState]); /* Keeps track of dependencies */

	//---------------------------- Start/End Coordinates --------------------------------

	/* Set starting state */
	const [startCoordinates_X, setStartCoordinates_X] = useState('12'); // TODO: Set to replay default
	const [startCoordinates_Y, setStartCoordinates_Y] = useState('42'); // TODO: Set to replay default
	const [endCoordinates_X, setEndCoordinates_X] = useState('73'); // TODO: Set to replay default
	const [endCoordinates_Y, setEndCoordinates_Y] = useState('13'); // TODO: Set to replay default

	/* Input Validation: checks if digit */
	const isDigit = (coordinate) => {
		return /^\d+$/.test(coordinate);
	}

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

		/* Draw circle at Start X, Y and End X, Y coordinates */
		renderPoints(ctx, startCoordinates_X, startCoordinates_Y, 'lime');
		renderPoints(ctx, endCoordinates_X, endCoordinates_Y, 'pink');
	  
		/* Re-renders map */
		renderBoundaries();
	};
	
	/* Draws lane boundaries given data from rover */
	const renderBoundaries = () => {
		for (let i = 0; i < SliderValue; i++) {
			draw([i,i], 135, 30, 30); // TODO: Connect to fetched
		}
	};

	/* Redraw canvas when change in current replay position or coordinates */
	useEffect(() => {
		drawOnCanvas();
	}, [SliderValue, startCoordinates_X, startCoordinates_Y, endCoordinates_X, endCoordinates_Y]);

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
	function draw(position, orientation, tofleft, tofright) {
		const canvas = document.querySelector('#canvas');
		if (canvas.getContext) {
			const ctx = canvas.getContext('2d');
			let l = 5; /* Change to alter length of each line */
			let theta = orientation*Math.PI/180
			/* Draw */
			drawLine(ctx, [position[0] + tofleft * Math.cos(theta), position[1] + tofleft * Math.sin(theta)], [position[0] + tofleft * Math.cos(theta) + l * Math.sin(theta), position[1] + tofleft * Math.sin(theta) - l * Math.cos(theta)]);
			drawLine(ctx, [position[0] - tofright * Math.cos(theta), position[1] - tofright * Math.sin(theta)], [position[0] - tofright * Math.cos(theta) + l * Math.sin(theta), position[1] - tofright * Math.sin(theta) - l * Math.cos(theta)]);
		}
	}

	//---------------------------- Mouse click coordinate select ------------------------

	/* Button onClick: start select */
	const handleSelectStart = () => {
		console.log('User select start');

		/* Add event listener to the canvas for start point selection */
		const canvas = canvasRef.current;
		canvas.addEventListener('click', handleCanvasClickStart);
	};
	
	/* Canvas onClick (start) */
	const handleCanvasClickStart = (event) => {
		const canvas = canvasRef.current;
		const rect = canvas.getBoundingClientRect();
	  
		/* Calculate the mouse position relative to the canvas */
		const x = event.clientX - rect.left;
		const y = event.clientY - rect.top;
	  
		/* Update the start coordinates with the mouse position */
		setStartCoordinates_X(String(Math.round(x)));
		setStartCoordinates_Y(String(Math.round(y)));
	  
		/* Remove the event listener after selecting the start point */
		canvas.removeEventListener('click', handleCanvasClickStart);
	};
	  
	/* Button onClick: end select */
	const handleSelectEnd = () => {
		console.log('User select end');
	  
		/* Add event listener to the canvas for end point selection */
		const canvas = canvasRef.current;
		canvas.addEventListener('click', handleCanvasClickEnd);
	};
	
	/* Canvas onClick (end) */
	const handleCanvasClickEnd = (event) => {
		const canvas = canvasRef.current;
		const rect = canvas.getBoundingClientRect();
	  
		/* Calculate the mouse position relative to the canvas */
		const x = event.clientX - rect.left;
		const y = event.clientY - rect.top;
	  
		/* Update the end coordinates with the mouse position */
		setEndCoordinates_X(String(Math.round(x)));
		setEndCoordinates_Y(String(Math.round(y)));
	  
		/* Remove the event listener after selecting the end point */
		canvas.removeEventListener('click', handleCanvasClickEnd);
	};
	  
	//---------------------------- Display ----------------------------------------------

	return (
		<div className="background">
			<div className="wrapper_Replay">
				{/* MAC address */}
				<div className="box-nobackground DisplayMAC_Replay">
					{nickname} / {MAC}
				</div>
				{/* Menu button */}
				<div className='box-red MenuButton_Replay'>
					<Link to='/' className="page-link" draggable={false}>
						<button onClick={handleMenu} className='box-red buttons_Replay'>
							Menu
						</button>
					</Link>
				</div>
				{/* Display boxes */}
				<div className="box ShortestPathStartControls_Rewind"/>
				<div className="box ShortestPathEndControls_Rewind"/>
				{/* Display text */}
				<div className="box StartText_Replay">
					Start
				</div>
				{/* Display text */}
				<div className="box EndText_Replay">
					End
				</div>
				{/* User-select start button */}
				<div className='box-green SelectStartButton_Replay'>
					<button onClick={handleSelectStart} className='box-green buttons_Replay'>
						select
					</button>
				</div>
				{/* User-select end button */}
				<div className='box-green SelectEndButton_Replay'>
					<button onClick={handleSelectEnd} className='box-green buttons_Replay'>
						select
					</button>
				</div>
				{/* User-editable and display Start X coordinate box */}
				<div className="box SelectStartCoordinates_X_Replay">
					<input
						type="text"
						value={startCoordinates_X}
						onChange={(e) => setStartCoordinates_X(e.target.value)}
					/>
				</div>
				{/* User-editable and display Start Y coordinate box */}
				<div className="box SelectStartCoordinates_Y_Replay">
					<input
						type="text"
						value={startCoordinates_Y}
						onChange={(e) => setStartCoordinates_Y(e.target.value)}
					/>
				</div>
				{/* User-editable and display End X coordinate box */}
				<div className="box SelectEndCoordinates_X_Replay">
					<input
						type="text"
						value={endCoordinates_X}
						onChange={(e) => setEndCoordinates_X(e.target.value)}
					/>
				</div>
				{/* User-editable and display End Y coordinate box */}
				<div className="box SelectEndCoordinates_Y_Replay">
					<input
						type="text"
						value={endCoordinates_Y}
						onChange={(e) => setEndCoordinates_Y(e.target.value)}
					/>
				</div>
				{/* Calculate shortest path button */}
				<div className='box-green ShortestPathButton_Replay'>
					<button onClick={handleShortestPath} className='box-green buttons_Replay'>
						Shortest Path
					</button>
				</div>
				{/* Display boxes */}
				<div className="box PlaybackControls_Rewind"/>
				{/* Rewind button */}
				<div className='box-nobackground RewindButton_Replay'>
					<button onMouseDown={handleRewindStart} onMouseUp={handleRewindStop} className='box-green buttons_Replay'>
						<img
							src={RewindIcon}
							alt={"Rewind"}
							style={{
								width: '40px',
								height: '40px',
								objectFit: 'cover', /* Adjusts how the image fits within the container */
							}}
							draggable={false}
						/>
					</button>
				</div>
				{/* Pause / Play / Replay button */}
				<div className='box-nobackground PauseButton_Replay'>
					{ PlayState === "Pause" ? (
						<button onClick={ChangePlayState} className='box-green buttons_Replay'>
							<img
								src={PlayIcon}
								alt={"Play"}
								style={{
									width: '40px',
									height: '40px',
									objectFit: 'cover', /* Adjust how the image fits within the container */
								}}
								draggable={false}
							/>
						</button>
					) : (<></>)
					}
					{ PlayState === "Play" ? (
						<button onClick={ChangePlayState} className='box-green buttons_Replay'>
							<img
								src={PauseIcon}
								alt={"Pause"}
								style={{
									width: '40px',
									height: '40px',
									objectFit: 'cover', /* Adjust how the image fits within the container */
								}}
								draggable={false}
							/>
						</button>
					) : (<></>)
					}
					{ PlayState === "End" ? (
						<button onClick={ChangePlayState} className='box-green buttons_Replay'>
							<img
								src={ReplayIcon}
								alt={"Replay"}
								style={{
									width: '40px',
									height: '40px',
									objectFit: 'cover', /* Adjust how the image fits within the container */
								}}
								draggable={false}
							/>
						</button>
					) : (<></>)
					}
				</div>
				{/* Fastforward button */}
				<div className='box-nobackground FastforwardButton_Replay'>
					<button onMouseDown={handleFastforwardStart} onMouseUp={handleFastforwardStop} className='box-green buttons_Replay'>
						<img
							src={FastforwardIcon}
							alt={"Fastforward"}
							style={{
								width: '40px',
								height: '40px',
								objectFit: 'cover', /* Adjust how the image fits within the container */
							}}
							draggable={false}
						/>
					</button>
				</div>
				{/* Toggle loop button */}
				<div className='box-nobackground LoopButton_Replay'>
					<button onClick={ChangeRepeatToggleState} className='box-green buttons_Replay'>
						<img
							src={ RepeatToggleState ? RepeatEnabledIcon : RepeatIcon }
							alt={"Repeat"}
							style={{
								width: '40px',
								height: '40px',
								objectFit: 'cover', /* Adjust how the image fits within the container */
							}}
							draggable={false}
						/>
					</button>
				</div>
				{/* Display Map */}
				<div className="box Map_Replay">
					<canvas ref={canvasRef} id='canvas' width={715} height={715} />
				</div>
				{/* Display Data */}
				<div className="box Data_Replay">
					<p>Data</p>
					<p>Data</p>
				</div>
				{/* Slider */}
				<div className="box-nobackground Slider_Replay">
					<input
						type="range"
						min="1"
						max={SliderValueMax} // TODO: Change to number of entries in map table
						value={SliderValue}
						className="SliderElement_Replay"
						id="myRange"
						onChange={handleSliderChange}
					/>
				</div>
			</div>
		</div>
	);
};

export default Replay;