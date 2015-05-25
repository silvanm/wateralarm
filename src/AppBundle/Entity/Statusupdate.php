<?php

namespace AppBundle\Entity;

use Doctrine\ORM\Mapping as ORM;

/**
 * Statusupdate
 *
 * @ORM\Table()
 * @ORM\Entity(repositoryClass="AppBundle\Entity\StatusupdateRepository")
 */
class Statusupdate
{
    /**
     * @var integer
     *
     * @ORM\Column(name="id", type="integer")
     * @ORM\Id
     * @ORM\GeneratedValue(strategy="AUTO")
     */
    private $id;

    /**
     * @var \DateTime
     *
     * @ORM\Column(name="createdAt", type="datetime")
     */
    private $createdAt;

    /**
     * @var integer
     *
     * @ORM\Column(name="waterlevel", type="integer")
     */
    private $waterlevel;

    /**
     * @var boolean
     *
     * @ORM\Column(name="alarmMuted", type="boolean")
     */
    private $alarmMuted;

	/**
	 * @var boolean
	 *
	 * @ORM\Column(name="isStatuschange", type="boolean")
	 */
	private $isStatuschange;

    /**
     * Get id
     *
     * @return integer 
     */
    public function getId()
    {
        return $this->id;
    }

    /**
     * Set createdAt
     *
     * @param \DateTime $createdAt
     * @return Statusupdate
     */
    public function setCreatedAt($createdAt)
    {
        $this->createdAt = $createdAt;

        return $this;
    }

    /**
     * Get createdAt
     *
     * @return \DateTime 
     */
    public function getCreatedAt()
    {
        return $this->createdAt;
    }

    /**
     * Set waterlevel
     *
     * @param integer $waterlevel
     * @return Statusupdate
     */
    public function setWaterlevel($waterlevel)
    {
        $this->waterlevel = $waterlevel;

        return $this;
    }

    /**
     * Get waterlevel
     *
     * @return integer 
     */
    public function getWaterlevel()
    {
        return $this->waterlevel;
    }

    /**
     * Set alarmMuted
     *
     * @param boolean $alarmMuted
     * @return Statusupdate
     */
    public function setAlarmMuted($alarmMuted)
    {
        $this->alarmMuted = $alarmMuted;

        return $this;
    }

    /**
     * Get alarmMuted
     *
     * @return boolean 
     */
    public function getAlarmMuted()
    {
        return $this->alarmMuted;
    }

	/**
	 * @return boolean
	 */
	public function isIsStatuschange() {
		return $this->isStatuschange;
	}

	/**
	 * @param boolean $isStatuschange
	 */
	public function setIsStatuschange( $isStatuschange ) {
		$this->isStatuschange = $isStatuschange;
	}


}
