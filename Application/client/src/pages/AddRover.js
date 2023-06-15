/*
Authors: Advik Chitre
Date created: 09/05/23
*/

//-------------------------------- Imports ----------------------------------------------

import React, { useState } from 'react';
import { Link } from 'react-router-dom';
import '../components/AddRover/grid_AddRover.css';
import '../components/grid.css';

//-------------------------------- Main -------------------------------------------------

/* 
Add Rover Page - User adds nickname for a given MAC address.
    	         This is posted to the server, which adds it to the database.
*/

const Home = () => {

    //---------------------------- Get Stored Values ------------------------------------

	const ServerIP = localStorage.getItem('ServerIP');

    //---------------------------- Create Variable  -------------------------------------

    const [inputMAC, setInputMAC] = useState('');
    const [inputNickname, setInputNickname] = useState('');

	//---------------------------- Button Functions: onClick ----------------------------

    /* handle when input changes */
    const handleNicknameChange = (event) => {
        setInputNickname(event.target.value);
    };
    const handleMACChange = (event) => {
        setInputMAC(event.target.value);
    };

    const handleSubmit = () => {
        console.log("Nickname added: " + inputNickname);
        console.log("MAC added: " + inputMAC);
        /* POST input to server */
        const AddRoverURL = "http://" + ServerIP + ":5000/client/rovernickname"; /* new rover endpoint */
        fetch(AddRoverURL, {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ "MAC": inputMAC, "nickname": inputNickname }) /* Gives sessionID, start coordinate to server */
		})
		.then(response => response.json())  /* Parse response as JSON */
		.then(data => {
			console.log("New Rover Response from server:", data);  /* Log response data */
		})
		/* Error handling */
		.catch(error => {
			console.log("Error:", error);
		});
    };

	//---------------------------- Display ----------------------------------------------

	return (
		<div className="background">
			<div className="wrapper">
				{/* Nickname */}
				<div className="box DisplayNickname_AddRover">
					Nickname			
				</div>
                <div className="box NicknameInput_AddRover">
                    <input type="text" value={inputNickname} onChange={handleNicknameChange} />	
				</div>

                {/* MAC */}
				<div className="box DisplayMAC_AddRover">
					MAC			
				</div>
                <div className="box MACInput_AddRover">
                    <input type="text" value={inputMAC} onChange={handleMACChange} />	
				</div>

				{/*Submit button */}
				<div className="box-green_AddRover Submit_AddRover">
                    <Link to='/' className="page-link" draggable={false}>
                        <button onClick={handleSubmit} className='buttons_AddRover'>
                            Submit
                        </button>
                    </Link>
				</div>
			</div>
		</div>
	);
};

export default Home;