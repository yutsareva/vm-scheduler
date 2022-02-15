CREATE SCHEMA IF NOT EXISTS tasks;

CREATE TYPE tasks.vm_status AS ENUM (
    'pending_allocation', 'allocating', 'allocated', 'pending_termination', 'terminating', 'terminated');

CREATE TABLE tasks.vms
(
    id                 serial PRIMARY KEY,
    status             tasks.vm_status          NOT NULL,

    cpu                integer                  NOT NULL, -- cores
    ram                integer                  NOT NULL, -- MB
    cpu_idle           integer                  NOT NULL, -- cores
    ram_idle           integer                  NOT NULL, -- MB

    cloud_vm_id        text,
    cloud_vm_type      text,

    created            timestamp with time zone NOT NULL,
    last_finished_task timestamp with time zone,
    agent_activity     timestamp with time zone
);

CREATE TYPE tasks.task_status AS ENUM (
    'queued', 'scheduled', 'running', 'completed', 'error', 'internal_error');

CREATE TABLE tasks.vm_statuses_log
(
    vm_id     integer REFERENCES tasks.vms (id),
    status    tasks.vm_status,
    timestamp timestamp with time zone NOT NULL
);

CREATE OR REPLACE FUNCTION update_vm_log() RETURNS TRIGGER AS
$$
BEGIN
    IF ((TG_OP = 'INSERT') OR (TG_OP = 'UPDATE') AND (OLD.status != NEW.status)) THEN
        IF (OLD.status != NEW.status) THEN
            INSERT INTO tasks.vm_statuses_log(vm_id, status, timestamp)
            VALUES (NEW.id, NEW.status, NOW());
        END IF;
    END IF;
    RETURN NULL;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER update_vm_log
    AFTER INSERT OR UPDATE
    ON tasks.vms
    FOR EACH ROW
EXECUTE PROCEDURE update_vm_log();

CREATE TABLE tasks.task_groups
(
    id        serial PRIMARY KEY,
    client_id text    NOT NULL,
    job_count integer NOT NULL
);


CREATE TABLE tasks.tasks
(
    id            serial PRIMARY KEY,
    status        tasks.task_status        NOT NULL,
    task_group_id integer                  NOT NULL REFERENCES tasks.task_groups (id),

    cpu           integer                  NOT NULL, -- cores
    ram           integer                  NOT NULL, -- MB
    estimation    interval                 NOT NULL,
    task_inputs   jsonb                    NOT NULL,
    task_outputs  jsonb,

    message       text,
    created       timestamp with time zone NOT NULL,
    started       timestamp with time zone,
    finished      timestamp with time zone,

    vm_id         integer REFERENCES tasks.vms (id)
);

CREATE TABLE tasks.task_statuses_log
(
    task_id   integer REFERENCES tasks.tasks (id),
    status    tasks.task_status,
    timestamp timestamp with time zone NOT NULL
);

CREATE OR REPLACE FUNCTION update_task_log() RETURNS TRIGGER AS
$$
BEGIN
    IF ((TG_OP = 'INSERT') OR (TG_OP = 'UPDATE') AND (OLD.status != NEW.status)) THEN
        IF (OLD.status != NEW.status) THEN
            INSERT INTO tasks.task_statuses_log(task_id, status, timestamp)
            VALUES (NEW.id, NEW.status, NOW());
        END IF;
    END IF;
    RETURN NULL;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER update_task_log
    AFTER INSERT OR UPDATE
    ON tasks.tasks
    FOR EACH ROW
EXECUTE PROCEDURE update_task_log();

CREATE TABLE tasks.plan
(
    id         serial PRIMARY KEY,
    created    timestamp with time zone NOT NULL,
    updated    timestamp with time zone NOT NULL,
    backend_id text
);
CREATE INDEX ON tasks.plan (updated);
