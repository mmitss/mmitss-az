#-----------------------------------------------------------------------------#
#    Dockerfile to build an image for M_PriorityRequestGenerator              #
#-----------------------------------------------------------------------------#
FROM mmitssuarizona/mmitss-base:1.1

MAINTAINER D Cunningham (donaldcunningham@email.arizona.edu)

COPY M_PriorityRequestGenerator/mmitss

CMD ./M_PriorityRequestGenerator
