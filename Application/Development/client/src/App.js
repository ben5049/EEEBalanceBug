import React from 'react';
import './App.css';
//import Navbar from './components/Navbar/NavbarElements';
//import { Navbar } from './components/Navbar';
import Navbar from './components/Navbar';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import Home from './pages';
import Replay from './pages/Replay';
import CompleteMap from './pages/CompleteMap';
import Connected from './pages/Connected';

function App() {
	return (
		<Router>
			<Navbar />
			<Routes>
				<Route exact path='/' exact element={<Home />} />
				<Route path='/finish' element={<CompleteMap />} />
				<Route path='/connected' element={<Connected />} />
				<Route path='/replay' element={<Replay />} />
			</Routes>
		</Router>
	);
}

export default App;
