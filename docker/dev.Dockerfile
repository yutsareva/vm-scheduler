FROM gcc:latest

COPY install.sh /
RUN sh install.sh

# User for CLion
RUN groupadd -g 712342 clion_user_group
RUN useradd -u 712342 -g 712342 -m clion_user && yes password | passwd clion_user

# SSH
EXPOSE 22
RUN echo "StrictHostKeyChecking=no" >> /etc/ssh/ssh_config
RUN mkdir /var/run/sshd

CMD ["/usr/sbin/sshd", "-D"]
