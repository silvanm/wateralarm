<?php

namespace AppBundle\Entity;

use Doctrine\ORM\EntityRepository;

/**
 * StatusupdateRepository
 *
 * This class was generated by the Doctrine ORM. Add your own custom
 * repository methods below.
 */
class StatusupdateRepository extends EntityRepository
{
	/**
	 * @return Statusupdate
	 */
	public function getStatus()
	{
		return $this->getEntityManager()
            ->createQuery(
                'SELECT s FROM AppBundle:Statusupdate s ORDER BY s.createdAt DESC'
            )
			->setMaxResults(1)
            ->getResult()[0];
	}

	/**
	 * @return Statusupdate
	 */
	public function getLastStatuschange()
		{
			return $this->getEntityManager()
	            ->createQuery(
	                'SELECT s FROM AppBundle:Statusupdate s WHERE s.isStatuschange = 1 ORDER BY s.createdAt DESC'
	            )
				->setMaxResults(1)
	            ->getResult()[0];
		}
}
