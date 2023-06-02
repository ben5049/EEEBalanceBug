import React from 'react';
import '../components/Connected/grid_Connected.css';
import '../components/grid.css';

const Connected = () => {
	const MAC = localStorage.getItem('MAC');
	const nickname = localStorage.getItem('nickname');
	console.log('CONNECTED MAC = ' + MAC)
	console.log('CONNECTED nickname = ' + nickname)

	return (
		<h1>ROVER DIAGNOSTICS</h1>
	);
};

export default Connected;
