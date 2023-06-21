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
import RoverImg from '../components/Connected/location.png';

//-------------------------------- Main -------------------------------------------------

/* 
Replay Page - given a replayID, this page displays a replay while also giving the user control.
    		  It also allows the user to change the shortest path start and end points.
*/

const Replay = () => {

	//---------------------------- Get Stored Values ------------------------------------

	const ServerIP = localStorage.getItem('ServerIP');
	const replayID = localStorage.getItem('ReplayID');
	console.log('Replay ID = ' + replayID);
	const MAC = localStorage.getItem('MAC');
	console.log('CONNECTED MAC = ' + MAC);
	const nickname = localStorage.getItem('nickname');
	console.log('CONNECTED nickname = ' + nickname);

	//---------------------------- Get Mapping Data -------------------------------------

	/* 
	Mapping - Gives to server: session id
			  Gets from server: map data 
	*/

	/* Create updating variables */
	const [MapData, UpdateMappingData] = useState({0 : [100, 100, 100, 90, 100, 100], 2 : [110, 100, 100, 90, 100, 100]});

	/* Send POST request to get Map Data */
	const MappingURL = "http://" + ServerIP + ":5000/client/replay";
	useEffect(() => {
		fetch(MappingURL, {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ "sessionid": replayID })
		})
		.then(response => response.json()) /* Parse the response as JSON */
		.then(data => UpdateMappingData(data)) /* Update the state with the data */
		.catch(error => console.log(error)); /* Error Handling */
	}, []);

	/* Get max magnitude in MapData when changed */
	const [MapDataMax, setMapDataMax] = useState(0);/* Number of Replay values (in MapData) */
	useEffect(() => {
		Object.entries(MapData).forEach(([key, value]) => {
			const firstTwoElements = value.slice(0, 2);
			const CoordinateMax = Math.max(...firstTwoElements);
			if (CoordinateMax > MapDataMax) {
				setMapDataMax(CoordinateMax);
				console.log("MapDataMax = " + CoordinateMax);
			}
		})
	}, [MapData]);

	/* Get largest magnitude negative number in MapData when changed */
	const [MapDataMin, setMapDataMin] = useState(0);/* Number of Replay values (in MapData) */
	useEffect(() => {
		Object.entries(MapData).forEach(([key, value]) => {
			const firstTwoElements = value.slice(0, 2);
			const CoordinateMin = Math.min(...firstTwoElements);
			if (CoordinateMin < MapDataMin) {
				setMapDataMin(CoordinateMin);
				console.log("MapDataMin = " + CoordinateMin);
			}
		})
	}, [MapData]);

	//---------------------------- Shortest Path ----------------------------------------

	/* Create updating variables */
	const [ShortestPath, UpdateShortestPath] = useState({0:{mapsto: -1, xcoord: 1, ycoord: 1}, 1:{mapsto: -1, xcoord: 10, ycoord: 10}}); /* Initially no shortest path selected */
	const [ShowShortestPath, UpdateShowShortestPath] = useState(false); /* Initially no shortest path displayed */

	/* Calculate shortest path */
	const handleShortestPath = async () => {
		console.log('Shortest Path');
		UpdateShowShortestPath(true);
		/* Input Validation: Check if start and end coordinates are digits */
		// if (!isDigit(startCoordinates_X) || !isDigit(startCoordinates_Y) || !isDigit(endCoordinates_X) || !isDigit(endCoordinates_Y)) {
		// 	console.log('Invalid coordinates.');
		// 	alert('Invalid coordinates. Please enter digits.');
		// 	return;
		// }
		await postStart(startCoordinates_X, startCoordinates_Y);
	};

	/* Visualise shortest path when shortest path changes */
	useEffect(() => {
		renderShortestPath();
	}, [ShortestPath]);

	/* Sends request to server for shortest path to every node from a given point (arguments: MAC, Start coordinates) */
	const postStart = async (StartX, StartY) => {
		const ShortestPathURL = "http://" + ServerIP + ":5000/client/shortestpath";
		console.log("URL = " + ShortestPathURL);

		try {
			/* Sends POST request to server */
			const response = await fetch(ShortestPathURL, {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ "sessionid": replayID, "start_x": StartX, "start_y": StartY }) /* Gives sessionID, start coordinate to server */
			});

			const data = await response.json(); /* Parse response as JSON */
			console.log("Response from server:", data); /* Log response data */
			UpdateShortestPath(data);
		} catch (error) {
			/* Error handling */
		  	console.log("Error:", error);
		}
	  };

	/* Draws a circle at coordinates */
	const renderPoints = (ctx, x, y, color) => {
		ctx.strokeStyle = color;
		ctx.lineWidth = 2;

		/* Scale to canvas */
		x  = ScaleToCanvas(x)
		y  = ScaleToCanvas(y)
	  
		/* Draw a circle */
		ctx.beginPath();
		ctx.arc(x, y, 4, 0, 2 * Math.PI); /* Radius = 4 */
		ctx.stroke();
	};

	const renderShortestPath = () => {
		// TODO: add for all nodes in ShortestPath
		
		Object.entries(ShortestPath).forEach((node) => {
			console.log("Shortest Path: " + node);
		});
		console.log("HERE");
		
		/* Find nearest node to end coordinate */
		let ClosestNode = 0;
		let minDistance = Infinity;
		Object.entries(ShortestPath).forEach((node) => {
			const distance = CalcDistance(node[1].xcoord, node[1].ycoord, endCoordinates_X, endCoordinates_Y);
			console.log(distance);
			if (distance < minDistance){
				ClosestNode = node[0];
				minDistance = distance;
			}
		});
		const ClosestNodeObject = ShortestPath[ClosestNode]
		console.log(ClosestNodeObject);
		console.log("Closest Node: " + ClosestNode);
		console.log("minDistance: " + minDistance);
		/* Snap end coordinate to nearest node */
		setEndCoordinates_X(ClosestNodeObject.xcoord);
		setEndCoordinates_Y(ClosestNodeObject.ycoord);

		/* Draw path to next node in predecessor graph.
		   Follow till -1 or length of graph */
		let CurrentNode = ClosestNodeObject;
		let NextNode = ShortestPath[CurrentNode.mapsto] /* Only maps to single value as only 1 closest path */
		while (CurrentNode.mapsto != -1) {
			/* Draw */
			console.log("While Looped")
			renderShortestPathSegment(CurrentNode.xcoord, CurrentNode.ycoord, NextNode.xcoord, NextNode.ycoord);
			/* Move to next node pair */
			CurrentNode = NextNode;
			NextNode = ShortestPath[CurrentNode.mapsto];
		}
		
		let start_x = CurrentNode.xcoord;
		let start_y = CurrentNode.ycoord;
		/* Snap start coordinate to last */
		setStartCoordinates_X(start_x);
		setStartCoordinates_Y(start_y);
	}

	/* Draw straight line between two nodes */
	const renderShortestPathSegment = (x1, y1, x2, y2) => {
		const canvas = canvasRef.current;
		const ctx = canvas.getContext('2d');
		ctx.beginPath();
		/* Scale */
		x1  = ScaleToCanvas(x1)
		y1  = ScaleToCanvas(y1)
		x2  = ScaleToCanvas(x2)
		y2  = ScaleToCanvas(y2)
		ctx.moveTo(x1, y1);
		ctx.lineTo(x2, y2);
		ctx.strokeStyle = "yellow"; // Set the line color
		ctx.setLineDash([5,5]); // Set the dash segments
		ctx.stroke();
		ctx.setLineDash([])
	}

	const CalcDistance = (x1, y1, x2, y2) => {
		// Pythagoras' theorem to find the distance between two points
		console.log(x1, y1, x2, y2)
		return Math.sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2);
	};

	//---------------------------- Button Functions: onClick ----------------------------
	
	/* Menu button */
	const handleMenu = () => {
		console.log('Menu');
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
	const SliderRate = 40; /* Time Interval (ms) */

	/* sliderValueMax updates when MapData is changed */
	const [SliderValueMax, setSliderValueMax] = useState(100);/* Number of Replay values (in MapData) */
	useEffect(() => {
		setSliderValueMax(Object.keys(MapData).length);
		console.log("Slider Value Max = " + Object.keys(MapData).length);
	}, [MapData]);


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
					if (newValue >= SliderValueMax) {
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
	const [startCoordinates_X, setStartCoordinates_X] = useState('3'); // TODO: Set to first MapData
	const [startCoordinates_Y, setStartCoordinates_Y] = useState('3'); // TODO: Set to first MapData
	const [endCoordinates_X, setEndCoordinates_X] = useState('700'); // TODO?: Set to replay default
	const [endCoordinates_Y, setEndCoordinates_Y] = useState('700'); // TODO?: Set to replay default

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
		renderPoints(ctx, endCoordinates_X, endCoordinates_Y, 'magenta');
	  
		/* Re-renders map */
		//console.log(MapData);
		renderBoundaries();

		/* display shortest path if button was pressed */
		if (ShowShortestPath) {
			renderShortestPath();
		}
		console.log("showShortestPath = " + ShowShortestPath);
	};

	/* Removes shortest path when coordinates changed */
	useEffect(() => {
		UpdateShowShortestPath(false);
	}, [startCoordinates_X, startCoordinates_Y, endCoordinates_X, endCoordinates_Y]);
	
	/* Draws lane boundaries given data from rover */
	const renderBoundaries = () => {
		const entries = Object.entries(MapData);
		let pos_x;
		let pos_y
		for (let i = 0; i < SliderValue && i < entries.length; i++) {
			const [, entry] = entries[i];
			console.log("#####################")
			console.log("X: " + entry[0] + " Y: " + entry[1] + " Angle: " + entry[3] + " TOF_L: " + entry[4] + " TOF_R: " + entry[5]);
			pos_x = -(entry[0]);
			pos_y = entry[1];
			console.log("PRESCALE -- x: " + pos_x + ", y: " + pos_y)
			const orientation = entry[3];
			const TOF_left = entry[4];
			const TOF_right = entry[5];
			draw(pos_x, pos_y, -orientation, TOF_left, TOF_right);
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
			context.drawImage(image, ScaleToCanvas(x), ScaleToCanvas(y), 21/3, 32/3);
    	};
	}

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
	function draw(position_x, position_y, orientation, tofleft, tofright) {
		const canvas = document.querySelector('#canvas');
		if (canvas.getContext) {
			const ctx = canvas.getContext('2d');
			let l = 5; /* Change to alter length of each line */
			let theta = orientation*Math.PI/180
			/* Scale */
			console.log("PRESCALE DRAWING LINE FROM x: " + position_x + ", y: " + position_y)
			position_x  = parseFloat(ScaleToCanvas(position_x))
			position_y  = parseFloat(ScaleToCanvas(position_y))
			/* Draw and reject out of bounds */
			console.log("DRAWING LINE FROM x: " + position_x + ", y: " + position_y)
			if (tofleft < 800){
				tofleft  = tofleft / (MapDataMax - MapDataMin + 1600)  * 700
				console.log("LEFT DASH START X   = " + (position_x + tofleft * Math.cos(theta)) + "    - position_x: " + position_x, ", tofleft: " + tofleft + ", Math.cos(theta)" + Math.cos(theta))
				console.log("LEFT DASH START Y   = " + position_y + tofleft * Math.sin(theta))
				drawLine(ctx, [position_x + tofleft * Math.cos(theta), position_y + tofleft * Math.sin(theta)], [position_x + tofleft * Math.cos(theta) + l * Math.sin(theta), position_y + tofleft * Math.sin(theta) - l * Math.cos(theta)]);
			} 
			if (tofright < 800){
				tofright  = tofright / (MapDataMax - MapDataMin + 1600)  * 700
				console.log("RIGHT DASH START = " + (position_x - tofright * Math.cos(theta)) + "    - position_x: " + position_x, ", tofright: " + tofright + ", Math.cos(theta)" + Math.cos(theta))
				drawLine(ctx, [position_x - tofright * Math.cos(theta), position_y - tofright * Math.sin(theta)], [position_x - tofright * Math.cos(theta) + l * Math.sin(theta), position_y - tofright * Math.sin(theta) - l * Math.cos(theta)]);
			}
		}
	}

	/* Scale to canvas as coordinate system for rover different to canvas */
	const ScaleToCanvas = (coordinate) => {
		//console.log("Min = " + MapDataMin + "  Max = " + MapDataMax)
		const out = ( (coordinate - MapDataMin + 800) / (MapDataMax - MapDataMin + 1600) ) * 700 + 15
		console.log("SCALE ==> IN: " + coordinate + " OUT: " + out)
		return out /* add offset so not touching border */
	}

	/* Scale to canvas as coordinate system for rover different to canvas */
	const DescaleToCanvas = (coordinate) => {
		return (( coordinate - 15) / 700 ) * (MapDataMax - MapDataMin + 1600) + MapDataMin - 800 /* add offset so not touching border */
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
		setStartCoordinates_X(String(Math.round(DescaleToCanvas(x))));
		setStartCoordinates_Y(String(Math.round(DescaleToCanvas(y))));
	  
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
		setEndCoordinates_X(String(Math.round(DescaleToCanvas(x))));
		setEndCoordinates_Y(String(Math.round(DescaleToCanvas(y))));
	  
		/* Remove the event listener after selecting the end point */
		canvas.removeEventListener('click', handleCanvasClickEnd);
	};

	//---------------------------- Get Diagnostic Data ----------------------------------

	/* 
	Diagnostic - Gives to server: session id
			  	 Gets from server: diagnostic data 
	*/

	/* Create updating variables */
	const [DiagnosticData, UpdateDiagnosticData] = useState([]);

	/* Send POST request to get Map Data */
	const DiagnosticURL = "http://" + ServerIP + ":5000/client/diagnostics";
	useEffect(() => {
		fetch(DiagnosticURL, {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ "sessionid": replayID })
		})
		.then(response => response.json()) /* Parse the response as JSON */
		.then(data => UpdateDiagnosticData(data)) /* Update the state with the data */
		.catch(error => console.log(error)); /* Error Handling */
	}, []);

	/* Update diagnostic data whenever slider value changes */
	const [DiagnosticData_Current, UpdateDiagnosticData_Current] = useState([]);
	useEffect(() => {
		const DiagnosticData_Current = DiagnosticData[SliderValue];
	}, [SliderValue]);

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
					CPU: {DiagnosticData_Current.CPU}, MAC: {DiagnosticData_Current.MAC}, Battery: {DiagnosticData_Current.battery}, Connection: {DiagnosticData_Current.connection}, Timestamp: {DiagnosticData_Current.timestamp}
				</div>
				{/* Slider */}
				<div className="box-nobackground Slider_Replay">
					<input
						type="range"
						min="1"
						max={SliderValueMax}
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