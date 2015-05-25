<?php

namespace AppBundle\Controller;

use Sensio\Bundle\FrameworkExtraBundle\Configuration\Route;
use Sensio\Bundle\FrameworkExtraBundle\Configuration\Template;
use Symfony\Bundle\FrameworkBundle\Controller\Controller;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\Response;
use AppBundle\Entity\Statusupdate;
use Symfony\Component\Validator\Constraints\DateTime;


class DefaultController extends Controller {

	/**
	 * @Route("/", name="statuspage")
	 * @Template
	 */
	public function indexAction() {
		return [ ];
	}

	/**
	 * Sends the statusupdates to the browser
	 *
	 * @Route("/get-status-json", name="getStatusJson")
	 */
	public function statusJsonAction() {

		/** @var Statusupdate $status */
		$status = $this->getDoctrine()->getRepository( "AppBundle:Statusupdate" )->getStatus();

		/** @var Statusupdate $statuschange */
		$statuschange = $this->getDoctrine()->getRepository( "AppBundle:Statusupdate" )->getLastStatuschange();


		switch ( $status->getWaterlevel() ) {
			case 0:
				$text         = 'OK.';
				$class        = 'ok';
				$buzzerstatus = 'silent';
				break;
			case 1:
			case 2:
				$text         = 'Warning';
				$class        = 'warning';
				$buzzerstatus = $status->getAlarmMuted() ? 'muted' : 'signaling';
				break;
			case 3:
				$text         = 'ALARM!';
				$class        = 'alarm';
				$buzzerstatus = $status->getAlarmMuted() ? 'muted' : 'signaling';

		}

		$response = new Response( json_encode(
			[
				'text'         => $text,
				'class'        => $class,
				'lastUpdate'   => $status->getCreatedAt(),
				'lastChange'   => $statuschange->getCreatedAt(),
				'buzzerStatus' => $buzzerstatus
			]

		) );
		$response->headers->set( 'Content-Type', 'application/json' );

		return $response;
	}

	/**
	 * Receives the status update from the arduino
	 *
	 * Protocol:
	 * waterlevel = 0-3 (water level)
	 * muted = 0/1 (whether the alarm is muted by a user
	 *
	 * @Route("/post-status", name="poststatus")
	 */
	public function statusAction( Request $request ) {

		// check if it's a status change
		/** @var Statusupdate $status */
		$status = $this->getDoctrine()->getRepository( "AppBundle:Statusupdate" )->getStatus();

		$waterlevel = $request->query->get( 'waterlevel' );

		$statusupdate = new Statusupdate();
		$statusupdate->setCreatedAt( new \DateTime() );
		$statusupdate->setWaterlevel( $waterlevel );
		$statusupdate->setAlarmMuted( $request->query->get( 'muted' ) == 1 );
		$statusupdate->setIsStatuschange( $status->getWaterlevel() != $waterlevel );

		$orm = $this->getDoctrine()->getManager();
		$orm->persist( $statusupdate );
		$orm->flush();

		return new Response( "Status received" );
	}
}
