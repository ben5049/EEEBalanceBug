import React, { useState } from 'react';
import Slider from '../components/Replay/Slider';
import '../components/Replay/grid_Replay.css';
import '../components/grid.css';

const Replay = () => {
	const ID = localStorage.getItem('ReplayID');
	console.log('Replay ID = ' + ID)
	
	return (
		<div>
			<h1>
                display Replay
            </h1>
			<Slider /> {/* Replace the "display Replay" with the RangeSlider component */}
		</div>
	);
};

export default Replay;
