CREATE SCHEMA IF NOT EXISTS scheduler;


-- ************************************ VMs table ************************************

CREATE TYPE scheduler.vm_status AS ENUM (
    'pending_allocation', 'allocating', 'allocated', 'agent_started',
    'pending_termination', 'terminating', 'terminated');

CREATE TABLE scheduler.vms
(
    id                 serial PRIMARY KEY,
    status             scheduler.vm_status      NOT NULL,

    cpu                integer                  NOT NULL, -- cores
    ram                integer                  NOT NULL, -- MB
    cpu_idle           integer                  NOT NULL, -- cores
    ram_idle           integer                  NOT NULL, -- MB

    cloud_vm_id        text,
    cloud_vm_type      text,

    created            timestamp with time zone NOT NULL,
    last_status_update timestamp with time zone NOT NULL,
    agent_activity     timestamp with time zone,

    restart_count      integer                  NOT NULL DEFAULT 0
);

CREATE OR REPLACE FUNCTION update_last_status_update() RETURNS TRIGGER AS
$$
BEGIN
    IF ((TG_OP = 'UPDATE') AND (OLD.status != NEW.status)) THEN
        UPDATE scheduler.vms
        SET last_status_update = NOW()
        WHERE id = NEW.id;
    END IF;
    RETURN NULL;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER update_last_status_update
    AFTER INSERT OR UPDATE
    ON scheduler.vms
    FOR EACH ROW
EXECUTE PROCEDURE update_last_status_update();


-- ************************************ VM statuses log table ************************

CREATE TABLE scheduler.vm_statuses_log
(
    vm_id     integer REFERENCES scheduler.vms (id),
    status    scheduler.vm_status,
    timestamp timestamp with time zone NOT NULL
);

CREATE OR REPLACE FUNCTION update_vm_log() RETURNS TRIGGER AS
$$
BEGIN
    IF ((TG_OP = 'INSERT') OR ((TG_OP = 'UPDATE') AND (OLD.status != NEW.status))) THEN
        INSERT INTO scheduler.vm_statuses_log(vm_id, status, timestamp)
        VALUES (NEW.id, NEW.status, NOW());
    END IF;
    RETURN NULL;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER update_vm_log
    AFTER INSERT OR UPDATE
    ON scheduler.vms
    FOR EACH ROW
EXECUTE PROCEDURE update_vm_log();


-- ************************************ Tasks table **********************************

CREATE TABLE scheduler.tasks
(
    id            serial PRIMARY KEY,
    job_count     integer NOT NULL,
    settings      jsonb   NOT NULL,
    image_version text    NOT NULL,
    client_id     text    NOT NULL
);


-- ************************************ Jobs table ***********************************

CREATE TYPE scheduler.job_status AS ENUM (
    'queued', 'scheduled', 'running', 'completed', 'error', 'internal_error', 'cancelling', 'cancelled');

CREATE TABLE scheduler.jobs
(
    id            serial PRIMARY KEY,
    status        scheduler.job_status     NOT NULL,
    task_id       integer                  NOT NULL REFERENCES scheduler.tasks (id),

    cpu           integer                  NOT NULL, -- cores
    ram           integer                  NOT NULL, -- MB
    estimation    interval                 NOT NULL,
    options       jsonb                    NOT NULL,
    result_url    text,

    message       text,
    created       timestamp with time zone NOT NULL,
    started       timestamp with time zone,
    finished      timestamp with time zone,

    vm_id         integer REFERENCES scheduler.vms (id),

    restart_count integer                  NOT NULL DEFAULT 0
);


-- ************************************ Job statuses log table ***********************

CREATE TABLE scheduler.job_statuses_log
(
    job_id    integer REFERENCES scheduler.jobs (id),
    status    scheduler.job_status,
    timestamp timestamp with time zone NOT NULL
);

CREATE OR REPLACE FUNCTION update_job_log() RETURNS TRIGGER AS
$$
BEGIN
    IF ((TG_OP = 'INSERT') OR ((TG_OP = 'UPDATE') AND (OLD.status != NEW.status))) THEN
        INSERT INTO scheduler.job_statuses_log(job_id, status, timestamp)
        VALUES (NEW.id, NEW.status, NOW());
    END IF;
    RETURN NULL;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER update_job_log
    AFTER INSERT OR UPDATE
    ON scheduler.jobs
    FOR EACH ROW
EXECUTE PROCEDURE update_job_log();


-- ************************************ Plan table ***********************************

CREATE TABLE scheduler.plan
(
    id          serial PRIMARY KEY,
    created     timestamp with time zone NOT NULL,
    updated     timestamp with time zone NOT NULL,
    lock_number bigint,
    backend_id  text
);
CREATE INDEX ON scheduler.plan (updated);
