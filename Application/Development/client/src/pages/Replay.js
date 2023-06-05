import React, { useState, useEffect } from 'react';
import { Link } from 'react-router-dom';
import Slider from '../components/Replay/Slider';
import '../components/Replay/grid_Replay.css';
import '../components/grid.css';
import RewindIcon from '../components/Replay/RewindIcon.png';
import PauseIcon from '../components/Replay/PauseIcon.png';
import PlayIcon from '../components/Replay/PlayIcon.png';
import ReplayIcon from '../components/Replay/ReplayIcon.png';
import FastforwardIcon from '../components/Replay/FastforwardIcon.png';
import RepeatIcon from '../components/Replay/RepeatIcon.png';
import RepeatEnabledIcon from '../components/Replay/RepeatEnabledIcon.png';


const Replay = () => {
	const ID = localStorage.getItem('ReplayID');
	const MAC = localStorage.getItem('MAC');
	const nickname = localStorage.getItem('nickname');
	console.log('Replay ID = ' + ID)
	console.log('CONNECTED MAC = ' + MAC)
	console.log('CONNECTED nickname = ' + nickname)
	
	
	// Button click handles
	const handleMenu = () => {
		console.log('Menu');
	};
	const handleSelectStart = () => {
		console.log('User select start');
	};
	const handleSelectEnd = () => {
		console.log('User select end');
	};
	const handleShortestPath = () => {
		console.log('Shortest Path');
	};
	const handleRewind = () => {
		console.log('Rewind');
	};
	const handlePause = () => {
		console.log('Pause');
	};
	const handlePlay = () => {
		console.log('Play');
	};
	const handleReplay = () => {
		console.log('Replay');
		setSliderValue(parseInt(0));
	};
	const handleFastforward = () => {
		console.log('Fast Forward');
	};
	const handleRepeat = () => {
		console.log('Repeat');
	};

	// Get starting state
	const [PlayState, setPlayState] = useState('Pause');
	// Switch state on button click
	const ChangePlayState = () => {
		if (PlayState == "Pause") {
			setPlayState("Play");
			handlePlay();
		}
		else if (PlayState == "Play") {
			setPlayState("Pause");
			handlePause();
		}
		else if (PlayState == "End") {
			setPlayState("Play");
			handleReplay();
		}
	};

	// Get starting state
	const [RepeatToggleState, setRepeatToggleState] = useState(false);
	// Switch state on button click
	const ChangeRepeatToggleState = () => {
		if (RepeatToggleState == true) {
			setRepeatToggleState(false);
		}
		else if (RepeatToggleState == false) {
			setRepeatToggleState(true);
		}
	};

	// Slider
	const [SliderValue, setSliderValue] = useState(0);
	const SliderValueMax = 100; // Number of Replay values, TODO: get from server
	const SliderRate = 40; // Time Interval (ms)

	const handleSliderChange = (event) => {
		setSliderValue(parseInt(event.target.value));
		if      (RepeatToggleState == false) {
			if (event.target.value == SliderValueMax) {
				setPlayState("End");
			}
			else {
				setPlayState("Pause");
			}
		}
		else {
			setPlayState("Pause");
		}
	};

	// Increment SliderValue every 0.1 seconds when PlayState is "Play"
	useEffect(() => {
		let interval = null;
	
		if (PlayState === "Play") {
			interval = setInterval(() => {
				setSliderValue((prevValue) => {
				const newValue = prevValue + 1;
				if      (RepeatToggleState == false) {
					if (newValue == SliderValueMax) {
						setPlayState("End");
					}
					return newValue > SliderValueMax ? SliderValueMax : newValue;
				}
				else if (RepeatToggleState == true) {
					return newValue > SliderValueMax ? 0 : newValue;
				}
				});
			}, SliderRate); 
		}

		return () => {
			clearInterval(interval);
		};
	}, [PlayState]);

	// User-editable Coordinates
	const [startCoordinates_X, setStartCoordinates_X] = useState(''); // TODO: Set to replay default
	const [startCoordinates_Y, setStartCoordinates_Y] = useState(''); // TODO: Set to replay default
	const [endCoordinates_X, setEndCoordinates_X] = useState(''); // TODO: Set to replay default
	const [endCoordinates_Y, setEndCoordinates_Y] = useState(''); // TODO: Set to replay default
	


	return (
		<div className="background">
			<div className="wrapper_Replay">
				<div className="box-nobackground DisplayMAC_Replay">
					{nickname} / {MAC}
				</div>
				<div className='box-red MenuButton_Replay'>
					<Link to='/' className="page-link" draggable={false}>
						<button onClick={handleMenu} className='box-red buttons_Replay'>
							Menu
						</button>
					</Link>
				</div>
				<div className="box ShortestPathStartControls_Rewind"/>
				<div className="box ShortestPathEndControls_Rewind"/>
				<div className="box StartText_Replay">
					Start
				</div>
				<div className="box EndText_Replay">
					End
				</div>
				<div className='box-green SelectStartButton_Replay'>
					<button onClick={handleSelectStart} className='box-green buttons_Replay'>
						select
					</button>
				</div>
				<div className='box-green SelectEndButton_Replay'>
					<button onClick={handleSelectEnd} className='box-green buttons_Replay'>
						select
					</button>
				</div>
				<div className="box SelectStartCoordinates_X_Replay">
					<input
						type="text"
						value={startCoordinates_X}
						onChange={(e) => setStartCoordinates_X(e.target.value)}
					/>
				</div>
				<div className="box SelectStartCoordinates_Y_Replay">
					<input
						type="text"
						value={startCoordinates_Y}
						onChange={(e) => setStartCoordinates_Y(e.target.value)}
					/>
				</div>
				<div className="box SelectEndCoordinates_X_Replay">
					<input
						type="text"
						value={endCoordinates_X}
						onChange={(e) => setEndCoordinates_X(e.target.value)}
					/>
				</div>
				<div className="box SelectEndCoordinates_Y_Replay">
					<input
						type="text"
						value={endCoordinates_Y}
						onChange={(e) => setEndCoordinates_Y(e.target.value)}
					/>
				</div>
				<div className='box-green ShortestPathButton_Replay'>
					<button onClick={handleShortestPath} className='box-green buttons_Replay'>
						Shortest Path
					</button>
				</div>
				<div className="box PlaybackControls_Rewind"/>	
				<div className='box-nobackground RewindButton_Replay'>
					<button onClick={handleRewind} className='box-green buttons_Replay'>
						<img
							src={RewindIcon}
							alt={"Rewind"}
							style={{
								width: '40px', // Set the desired width
								height: '40px', // Set the desired height
								objectFit: 'cover', // Adjust how the image fits within the container
							}}
							draggable={false}
						/>
					</button>
				</div>
				<div className='box-nobackground PauseButton_Replay'>
					{ PlayState == "Pause" ? (
						<button onClick={ChangePlayState} className='box-green buttons_Replay'>
							<img
								src={PlayIcon}
								alt={"Play"}
								style={{
									width: '40px', // Set the desired width
									height: '40px', // Set the desired height
									objectFit: 'cover', // Adjust how the image fits within the container
								}}
								draggable={false}
							/>
						</button>
					) : (<></>)
					}
					{ PlayState == "Play" ? (
						<button onClick={ChangePlayState} className='box-green buttons_Replay'>
							<img
								src={PauseIcon}
								alt={"Pause"}
								style={{
									width: '40px', // Set the desired width
									height: '40px', // Set the desired height
									objectFit: 'cover', // Adjust how the image fits within the container
								}}
								draggable={false}
							/>
						</button>
					) : (<></>)
					}
					{ PlayState == "End" ? (
						<button onClick={ChangePlayState} className='box-green buttons_Replay'>
							<img
								src={ReplayIcon}
								alt={"Replay"}
								style={{
									width: '40px', // Set the desired width
									height: '40px', // Set the desired height
									objectFit: 'cover', // Adjust how the image fits within the container
								}}
								draggable={false}
							/>
						</button>
					) : (<></>)
					}
				</div>
				<div className='box-nobackground FastforwardButton_Replay'>
					<button onClick={handleFastforward} className='box-green buttons_Replay'>
						<img
							src={FastforwardIcon}
							alt={"Fastforward"}
							style={{
								width: '40px', // Set the desired width
								height: '40px', // Set the desired height
								objectFit: 'cover', // Adjust how the image fits within the container
							}}
							draggable={false}
						/>
					</button>
				</div>
				<div className='box-nobackground LoopButton_Replay'>
					<button onClick={ChangeRepeatToggleState} className='box-green buttons_Replay'>
						<img
							src={ RepeatToggleState ? RepeatEnabledIcon : RepeatIcon }
							alt={"Repeat"}
							style={{
								width: '40px', // Set the desired width
								height: '40px', // Set the desired height
								objectFit: 'cover', // Adjust how the image fits within the container
							}}
							draggable={false}
						/>
					</button>
				</div>
				<div className="box Map_Replay">
					Map	
				</div>
				<div className="box Data_Replay">
					<p>Data</p><p>Data</p>
				</div>
				<div className="box-nobackground Slider_Replay">
						<input
							type="range"
							min="1"
							max="100" // TODO: Change to number of entries in map table
							value={SliderValue}
							className="SliderElement_Replay"
							id="myRange"
							onChange={handleSliderChange}
						/>
					{/*<p>Value: <span id="demo">{SliderValue}</span></p>*/}
				</div>
			</div>
		</div>
	);
};

export default Replay;
